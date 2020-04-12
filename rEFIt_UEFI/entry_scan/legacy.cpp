/*
 * refit/scan/legacy.c
 *
 * Copyright (c) 2006-2010 Christoph Pfisterer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *  * Neither the name of Christoph Pfisterer nor the names of the
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "entry_scan.h"
#include "../refit/screen.h"
#include "../refit/menu.h"

#ifndef DEBUG_ALL
#define DEBUG_SCAN_LEGACY 1
#else
#define DEBUG_SCAN_LEGACY DEBUG_ALL
#endif

#if DEBUG_SCAN_LEGACY == 0
#define DBG(...)
#else
#define DBG(...) DebugLog(DEBUG_SCAN_LEGACY, __VA_ARGS__)
#endif

//the function is not in the class and deals always with MainMenu
//I made args as pointers to have an ability to call with NULL
BOOLEAN AddLegacyEntry(IN const XStringW& FullTitle, IN const XStringW& LoaderTitle, IN REFIT_VOLUME *Volume, IN const XImage* Image, IN const XImage* DriveImage, IN CHAR16 Hotkey, IN BOOLEAN CustomEntry)
{
  LEGACY_ENTRY      *Entry, *SubEntry;
  REFIT_MENU_SCREEN *SubScreen;
  CONST CHAR16      *VolDesc;
  CHAR16             ShortcutLetter = 0;
//  INTN               i;
  
  if (Volume == NULL) {
    return false;
  }
  // Ignore this loader if it's device path is already present in another loader
    for (UINTN i = 0; i < MainMenu.Entries.size(); ++i) {
      REFIT_ABSTRACT_MENU_ENTRY& MainEntry = MainMenu.Entries[i];
//      DBG("entry %lld\n", i);
      // Only want legacy
      if (MainEntry.getLEGACY_ENTRY()) {
        if (StriCmp(MainEntry.getLEGACY_ENTRY()->DevicePathString, Volume->DevicePathString) == 0) {
          return  true;
        }
      }
    }

  // If this isn't a custom entry make sure it's not hidden by a custom entry
  if (!CustomEntry) {
    CUSTOM_LEGACY_ENTRY *Custom = gSettings.CustomLegacy;
    while (Custom) {
      if (OSFLAG_ISSET(Custom->Flags, OSFLAG_DISABLED) ||
          (OSFLAG_ISSET(Custom->Flags, OSFLAG_HIDDEN) && !gSettings.ShowHiddenEntries)) {
        if (Custom->Volume) {
          if ((StrStr(Volume->DevicePathString, Custom->Volume) == NULL) &&
              ((Volume->VolName == NULL) || (StrStr(Volume->VolName, Custom->Volume) == NULL))) {
            if (Custom->Type != 0) {
              if (Custom->Type == Volume->LegacyOS->Type) {
                return  false;
              }
            } else {
              return  false;
            }
          }
        } else if (Custom->Type != 0) {
          if (Custom->Type == Volume->LegacyOS->Type) {
            return  false;
          }
        }
      }
      Custom = Custom->Next;
    }
  }
  XStringW LTitle;
  if (LoaderTitle.isEmpty()) {
    if (Volume->LegacyOS->Name != NULL) {
      LTitle.takeValueFrom(Volume->LegacyOS->Name);
      if (Volume->LegacyOS->Name[0] == 'W' || Volume->LegacyOS->Name[0] == 'L')
        ShortcutLetter = Volume->LegacyOS->Name[0];
    } else
      LTitle = L"Legacy OS"_XSW;
  } else
    LTitle = LoaderTitle;
  if (Volume->VolName != NULL)
    VolDesc = Volume->VolName;
  else
    VolDesc = (Volume->DiskKind == DISK_KIND_OPTICAL) ? L"CD" : L"HD";
//DBG("VolDesc=%ls\n", VolDesc);

  // prepare the menu entry
  Entry = new LEGACY_ENTRY();
  if (!FullTitle.isEmpty()) {
    Entry->Title = FullTitle;
  } else {
    if (ThemeX.BootCampStyle) {
      Entry->Title = LTitle;
    } else {
      Entry->Title = L"Boot "_XSW + LoaderTitle + L" from "_XSW + VolDesc;
//    Entry->Title.SWPrintf("Boot %ls from %ls", LoaderTitle->wc_str(), VolDesc);
    }
  }

//  DBG("Title=%ls\n", Entry->Title);
  Entry->Row          = 0;
  Entry->ShortcutLetter = (Hotkey == 0) ? ShortcutLetter : Hotkey;

  if (Image) {
    Entry->Image = *Image;
  } else {
    Entry->Image = ThemeX.LoadOSIcon(Volume->LegacyOS->IconName);
    if (Entry->Image.isEmpty()) {
      Entry->Image = ThemeX.GetIcon("os_win"_XS); //we have no legacy.png
    }
  }

//  DBG("IconName=%ls\n", Volume->LegacyOS->IconName);

    Entry->DriveImage = (DriveImage != NULL) ? *DriveImage : ScanVolumeDefaultIcon(Volume, Volume->LegacyOS->Type, Volume->DevicePath);

  //  DBG("HideBadges=%d Volume=%ls\n", GlobalConfig.HideBadges, Volume->VolName);
  //  DBG("Title=%ls OSName=%ls OSIconName=%ls\n", LoaderTitle, Volume->OSName, Volume->OSIconName);
  //actions
  Entry->AtClick = ActionSelect;
  Entry->AtDoubleClick = ActionEnter;
  Entry->AtRightClick = ActionDetails;
  if (ThemeX.HideBadges & HDBADGES_SHOW) {
    if (ThemeX.HideBadges & HDBADGES_SWAP) {
      Entry->BadgeImage = XImage(Entry->DriveImage, ThemeX.BadgeScale/16.f); //0 accepted
    } else {
      Entry->BadgeImage = XImage(Entry->Image, ThemeX.BadgeScale/16.f);
    }
  }
  Entry->Volume           = Volume;
  Entry->DevicePathString = Volume->DevicePathString;
  Entry->LoadOptions      = (Volume->DiskKind == DISK_KIND_OPTICAL) ? "CD"_XS : ((Volume->DiskKind == DISK_KIND_EXTERNAL) ? "USB"_XS : "HD"_XS);
  // create the submenu
//  SubScreen = (__typeof__(SubScreen))AllocateZeroPool(sizeof(REFIT_MENU_SCREEN));
  SubScreen = new REFIT_MENU_SCREEN();
//  SubScreen->Title = L"Boot Options for "_XSW + LoaderTitle + L" on "_XSW + VolDesc;
	SubScreen->Title.SWPrintf("Boot Options for %ls on %ls", LoaderTitle.wc_str(), VolDesc);

  SubScreen->TitleImage = Entry->Image;
  SubScreen->AnimeRun = SubScreen->GetAnime();
  // default entry
  SubEntry =  new LEGACY_ENTRY();
  SubEntry->Title = L"Boot "_XSW + LoaderTitle;
//  SubEntry->Tag           = TAG_LEGACY;
  SubEntry->Volume           = Entry->Volume;
  SubEntry->DevicePathString = Entry->DevicePathString;
  SubEntry->LoadOptions      = Entry->LoadOptions;
  SubEntry->AtClick       = ActionEnter;
  SubScreen->AddMenuEntry(SubEntry, true);
  SubScreen->AddMenuEntry(&MenuEntryReturn, false);
  Entry->SubScreen = SubScreen;
  MainMenu.AddMenuEntry(Entry, true);
//  DBG(" added '%ls' OSType=%d Icon=%ls\n", Entry->Title, Volume->LegacyOS->Type, Volume->LegacyOS->IconName);
  return true;
}

VOID ScanLegacy(VOID)
{
  UINTN                   VolumeIndex, VolumeIndex2;
  BOOLEAN                 ShowVolume, HideIfOthersFound;
  REFIT_VOLUME            *Volume;
  
  DBG("Scanning legacy ...\n");
  
  for (VolumeIndex = 0; VolumeIndex < Volumes.size(); VolumeIndex++) {
    Volume = &Volumes[VolumeIndex];
//    DBG("test VI=%d\n", VolumeIndex);
    if ((Volume->BootType != BOOTING_BY_PBR) &&
        (Volume->BootType != BOOTING_BY_MBR) &&
        (Volume->BootType != BOOTING_BY_CD)) {
//      DBG(" not legacy\n");
      continue;
    }

//    DBG("%2d: '%ls' (%ls)", VolumeIndex, Volume->VolName, Volume->LegacyOS->IconName);
    
#if 0 // REFIT_DEBUG > 0
    DBG(" %d %ls\n  %d %d %ls %d %ls\n",
        VolumeIndex, FileDevicePathToStr(Volume->DevicePath),
        Volume->DiskKind, Volume->MbrPartitionIndex,
        Volume->IsAppleLegacy ? L"AL" : L"--", Volume->HasBootCode,
        Volume->VolName ? Volume->VolName : L"(no name)");
#endif
    
    // skip volume if its kind is configured as disabled
    if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & VOLTYPE_OPTICAL)) ||
        (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_EXTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_INTERNAL)) ||
        (Volume->DiskKind == DISK_KIND_FIREWIRE && (GlobalConfig.DisableFlags & VOLTYPE_FIREWIRE)))
    {
//      DBG(" hidden\n");
      continue;
    }
//    DBG("not hidden\n");
    ShowVolume = FALSE;
    HideIfOthersFound = FALSE;
    if (Volume->IsAppleLegacy) {
      ShowVolume = TRUE;
      HideIfOthersFound = TRUE;
    } else if (Volume->HasBootCode) {
      ShowVolume = TRUE;
//      DBG("Volume %d will be shown BlockIo=%X WholeIo=%X\n",
//        VolumeIndex, Volume->BlockIO, Volume->WholeDiskBlockIO);
      if ((Volume->WholeDiskBlockIO == 0) &&
          Volume->BlockIOOffset == 0 /* &&
                                      Volume->OSName == NULL */)
        // this is a whole disk (MBR) entry; hide if we have entries for partitions
        HideIfOthersFound = TRUE;
//      DBG("Hide it!\n");
    }
    if (HideIfOthersFound) {
      // check for other bootable entries on the same disk
      //if PBR exists then Hide MBR
      for (VolumeIndex2 = 0; VolumeIndex2 < Volumes.size(); VolumeIndex2++) {
//        DBG("what to hide %d\n", VolumeIndex2);
        if (VolumeIndex2 != VolumeIndex &&
            Volumes[VolumeIndex2].HasBootCode &&
            Volumes[VolumeIndex2].WholeDiskBlockIO == Volume->BlockIO){
          ShowVolume = FALSE;
//         DBG("PBR volume at index %d\n", VolumeIndex2);
          break;
        }
      }
    }
    
    if (ShowVolume && (!Volume->Hidden)){
//      DBG(" add legacy\n");
      if (!AddLegacyEntry(L""_XSW, L""_XSW, Volume, NULL, NULL, 0, FALSE)) {
        DBG("...entry not added\n");
      };
    } else {
      DBG(" hidden\n");
    }
  }
}

// Add custom legacy
VOID AddCustomLegacy(VOID)
{
  UINTN                VolumeIndex, VolumeIndex2;
  BOOLEAN              ShowVolume, HideIfOthersFound;
  REFIT_VOLUME        *Volume;
  CUSTOM_LEGACY_ENTRY *Custom;
  XImage Image;
  XImage DriveImage;
  UINTN                i = 0;
  
//  DBG("Custom legacy start\n");
  if (gSettings.CustomLegacy) {
    DbgHeader("AddCustomLegacy");
  }

  // Traverse the custom entries
  for (Custom = gSettings.CustomLegacy; Custom; ++i, Custom = Custom->Next) {
    if (OSFLAG_ISSET(Custom->Flags, OSFLAG_DISABLED)) {
      DBG("Custom legacy %llu skipped because it is disabled.\n", i);
      continue;
    }
    if (!gSettings.ShowHiddenEntries && OSFLAG_ISSET(Custom->Flags, OSFLAG_HIDDEN)) {
      DBG("Custom legacy %llu skipped because it is hidden.\n", i);
      continue;
    }
    if (Custom->Volume) {
      DBG("Custom legacy %llu matching \"%ls\" ...\n", i, Custom->Volume);
    }
    for (VolumeIndex = 0; VolumeIndex < Volumes.size(); ++VolumeIndex) {
      Volume = &Volumes[VolumeIndex];
      
      DBG("   Checking volume \"%ls\" (%ls) ... ", Volume->VolName, Volume->DevicePathString);
      
      // skip volume if its kind is configured as disabled
      if ((Volume->DiskKind == DISK_KIND_OPTICAL && (GlobalConfig.DisableFlags & VOLTYPE_OPTICAL)) ||
          (Volume->DiskKind == DISK_KIND_EXTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_EXTERNAL)) ||
          (Volume->DiskKind == DISK_KIND_INTERNAL && (GlobalConfig.DisableFlags & VOLTYPE_INTERNAL)) ||
          (Volume->DiskKind == DISK_KIND_FIREWIRE && (GlobalConfig.DisableFlags & VOLTYPE_FIREWIRE)))
      {
        DBG("skipped because media is disabled\n");
        continue;
      }
      
      if (Custom->VolumeType != 0) {
        if ((Volume->DiskKind == DISK_KIND_OPTICAL && ((Custom->VolumeType & VOLTYPE_OPTICAL) == 0)) ||
            (Volume->DiskKind == DISK_KIND_EXTERNAL && ((Custom->VolumeType & VOLTYPE_EXTERNAL) == 0)) ||
            (Volume->DiskKind == DISK_KIND_INTERNAL && ((Custom->VolumeType & VOLTYPE_INTERNAL) == 0)) ||
            (Volume->DiskKind == DISK_KIND_FIREWIRE && ((Custom->VolumeType & VOLTYPE_FIREWIRE) == 0))) {
          DBG("skipped because media is ignored\n");
          continue;
        }
      }
      if ((Volume->BootType != BOOTING_BY_PBR) &&
          (Volume->BootType != BOOTING_BY_MBR) &&
          (Volume->BootType != BOOTING_BY_CD)) {
        DBG("skipped because volume is not legacy bootable\n");
        continue;
      }
      
      ShowVolume = FALSE;
      HideIfOthersFound = FALSE;
      if (Volume->IsAppleLegacy) {
        ShowVolume = TRUE;
        HideIfOthersFound = TRUE;
      } else if (Volume->HasBootCode) {
        ShowVolume = TRUE;
        if ((Volume->WholeDiskBlockIO == 0) &&
            Volume->BlockIOOffset == 0) {
          // this is a whole disk (MBR) entry; hide if we have entries for partitions
          HideIfOthersFound = TRUE;
        }
      }
      if (HideIfOthersFound) {
        // check for other bootable entries on the same disk
        //if PBR exists then Hide MBR
        for (VolumeIndex2 = 0; VolumeIndex2 < Volumes.size(); VolumeIndex2++) {
          if (VolumeIndex2 != VolumeIndex &&
              Volumes[VolumeIndex2].HasBootCode &&
              Volumes[VolumeIndex2].WholeDiskBlockIO == Volume->BlockIO) {
            ShowVolume = FALSE;
            break;
          }
        }
      }
      
      if (!ShowVolume || (Volume->Hidden)) {
        DBG("skipped because volume is hidden\n");
        continue;
      }
      
      // Check for exact volume matches
      if (Custom->Volume) {
        if ((StrStr(Volume->DevicePathString, Custom->Volume) == NULL) &&
            ((Volume->VolName == NULL) || (StrStr(Volume->VolName, Custom->Volume) == NULL))) {
          DBG("skipped\n");
          continue;
        }
        // Check if the volume should be of certain os type
        if ((Custom->Type != 0) && (Custom->Type != Volume->LegacyOS->Type)) {
          DBG("skipped because wrong type\n");
          continue;
        }
      } else if ((Custom->Type != 0) && (Custom->Type != Volume->LegacyOS->Type)) {
        DBG("skipped because wrong type\n");
        continue;
      }
      // Change to custom image if needed
      Image = Custom->Image;
      if (Image.isEmpty()) {
        Image.LoadXImage(ThemeX.ThemeDir, Custom->ImagePath);
      }

      // Change to custom drive image if needed
      DriveImage = Custom->DriveImage;
      if (DriveImage.isEmpty()) {
        DriveImage.LoadXImage(ThemeX.ThemeDir, Custom->DriveImagePath);
      }
      // Create a legacy entry for this volume
      if (AddLegacyEntry(Custom->FullTitle, Custom->Title, Volume, &Image, &DriveImage, Custom->Hotkey, TRUE))
      {
        DBG("match!\n");
      }
    }
  }
  //DBG("Custom legacy end\n");
}
