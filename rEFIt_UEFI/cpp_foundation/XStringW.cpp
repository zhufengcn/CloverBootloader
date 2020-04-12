//*************************************************************************************************
//*************************************************************************************************
//
//                                      STRING
//
// Developed by jief666, from 1997.
//
//*************************************************************************************************
//*************************************************************************************************


#if !defined(__XStringW_CPP__)
#define __XStringW_CPP__

#if 0
#define DBG(...) DebugLog(2, __VA_ARGS__)
#else
#define DBG(...)
#endif

#include "XToolsCommon.h"
#include "XStringW.h"

#include "../../Include/Library/printf_lite.h"

UINTN XStringWGrowByDefault = 1024;
const XStringW NullXStringW;


void XStringW::Init(UINTN aSize)
{
//DBG("Init aSize=%d\n", aSize);
	m_data = (wchar_t*)malloc( (aSize+1)*sizeof(wchar_t) ); /* le 0 terminal n'est pas compté dans mSize */
	if ( !m_data ) {
		DebugLog(2, "XStringW::Init(%llu) : Xalloc returned NULL. Cpu halted\n", (aSize+1)*sizeof(wchar_t));
		panic();
	}
	m_allocatedSize = aSize;
	m_len = 0;
	m_data[0] = 0;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Constructor
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

XStringW::XStringW()
{
DBG("Construteur\n");
	Init();
}

XStringW::XStringW(const XStringW &aString)
{
DBG("Constructor(const XStringW &aString) : %ls\n", aString.data());
	Init(aString.size());
	StrnCpy(aString.data(), aString.size());
}
//
//XStringW::XStringW(const wchar_t *S)
//{
//	if ( !S ) {
//		DebugLog(2, "XStringW(const wchar_t *S) called with NULL. Use setEmpty()\n");
//		panic();
//	}
//DBG("Constructor(const wchar_t *S) : %ls, StrLen(S)=%d\n", S, StrLen(S));
//	Init(StrLen(S));
//	StrCpy(S);
//}
//
//XStringW::XStringW(const wchar_t *S, UINTN count)
//{
//DBG("Constructor(const wchar_t *S, UINTN count) : %ls, %d\n", S, count);
//	Init(count);
//	StrnCpy(S, count);
//}
//
//XStringW::XStringW(const wchar_t aChar)
//{
//DBG("Constructor(const wchar_t aChar)\n");
//	Init(1);
//	StrnCpy(&aChar, 1);
//}
//
//XStringW::XStringW(const char* S)
//{
//DBG("Constructor(const char* S)\n");
//	xsize newLen = StrLenInWChar(S, strlen(S));
//	Init(newLen);
//	utf8ToWChar(m_data, m_allocatedSize+1, S, strlen(S)); // m_size doesn't count the NULL terminator
//	SetLength(newLen);
//}
wchar_t * XStringW::forgetDataWithoutFreeing()
{
	wchar_t* ret = m_data;
	Init();
	return ret;
}

const XStringW& XStringW::takeValueFrom(const wchar_t* S, xsize count)
{
	if ( !S ) {
//		DebugLog(2, "takeValueFrom(const wchar_t* S) called with NULL. Use setEmpty()\n");
//		panic();
    Init(0);
    return *this;
	}
	Init(count);
	StrCpy(S);
	return *this;
}

const XStringW& XStringW::takeValueFrom(const wchar_t* S)
{
	if ( S ) return takeValueFrom(S, wcslen(S));
	Init(0);
        return *this;
}

const XStringW& XStringW::takeValueFrom(const char* S)
{
	xsize newLen = utf8_string_wchar_count(S);
	Init(newLen);
	wchar_string_from_utf8_string(m_data, m_allocatedSize+1, S); // m_size doesn't count the NULL terminator
	SetLength(newLen);
	return *this;
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Destructor
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx


XStringW::~XStringW()
{
DBG("Destructor :%ls\n", data());
	free((void*)m_data);
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void XStringW::SetLength(UINTN len)
{
//DBG("SetLength(%d)\n", len);
	CheckSize(len);

	m_len = len;
	m_data[len] = 0;

	if ( wcslen(data()) != len ) {
		DBG("XStringW::SetLength(UINTN len) : StrLen(data()) != len (%d != %d). System halted\n", StrLen(data()), len);
		panic();
	}
}

/* CheckSize() */
wchar_t *XStringW::CheckSize(UINTN nNewSize, UINTN nGrowBy)
{
//DBG("CheckSize: m_size=%d, nNewSize=%d\n", m_size, nNewSize);

	if ( m_allocatedSize < nNewSize )
	{
		nNewSize += nGrowBy;
		m_data = (wchar_t*)realloc(m_data, (nNewSize+1)*sizeof(wchar_t), m_allocatedSize*sizeof(wchar_t));
		if ( !m_data ) {
  		DBG("XStringW::CheckSize(%d, %d) : Xrealloc(%d, %d, %d) returned NULL. System halted\n", nNewSize, nGrowBy, m_size, (nNewSize+1)*sizeof(wchar_t), m_data);
	  	panic();
		}
		m_allocatedSize = nNewSize;
	}
	return m_data;
}

void XStringW::StrnCpy(const wchar_t *buf, UINTN len)
{
	UINTN newLen = 0;
	if ( buf && *buf && len > 0 ) {
		CheckSize(len, 0);
		while ( *buf && newLen < len ) {
			m_data[newLen++] = *buf++;
		}
//		memmove(data(), buf, len*sizeof(wchar_t));
	}
	SetLength(newLen); /* data()[len]=0 done in SetLength */
}

void XStringW::StrCpy(const wchar_t *buf)
{
	if ( buf && *buf ) {
		StrnCpy(buf, wcslen(buf));
	}else{
		SetLength(0); /* data()[0]=0 done in SetLength */
	}
}

void XStringW::StrnCat(const wchar_t *buf, UINTN len)
{
  UINTN NewLen;

	if ( buf && *buf && len > 0 ) {
		NewLen = size()+len;
		CheckSize(NewLen, 0);
		memmove(_data(size()), buf, len*sizeof(wchar_t));
		SetLength(NewLen); /* data()[NewLen]=0 done in SetLength */
	}
}

void XStringW::StrCat(const wchar_t *buf)
{
	if ( buf && *buf ) {
		StrnCat(buf, wcslen(buf));
	}
}

void XStringW::StrCat(const XStringW &uneXStringWW)
{
	StrnCat(uneXStringWW.data(), uneXStringWW.size());
}

void XStringW::Delete(UINTN pos, UINTN count)
{
	if ( pos < size() ) {
		if ( count != MAX_XSIZE  &&  pos + count < size() ) {
			memmove( _data(pos), data(pos+count), (size()-pos-count)*sizeof(wchar_t)); // memmove handles overlapping memory move
			SetLength(size()-count);/* data()[length()-count]=0 done in SetLength */
		}else{
			SetLength(pos);/* data()[pos]=0 done in SetLength */
		}
	}
}

void XStringW::Insert(UINTN pos, const XStringW& Str)
{
	if ( pos < size() ) {
		CheckSize(size()+Str.size());
		memmove(_data(pos + Str.size()),  data(pos),  (size()-pos)*sizeof(wchar_t));
		memmove(_data(pos), Str.data(), Str.size()*sizeof(wchar_t));
		SetLength(size()+Str.size());
	}else{
		StrCat(Str);
	}
}

void XStringW::Replace(wchar_t c1, wchar_t c2)
{
  wchar_t* p;

	p = _data(0);
	while ( *p ) {
		if ( *p == c1 ) *p = c2;
		p += 1;
	}
}

XStringW XStringW::SubStringReplace(wchar_t c1, wchar_t c2)
{
  wchar_t* p;
  XStringW Result;

	p = _data(0);
	while ( *p  ) {
		if ( *p == c1 ) Result += c2;
		else Result += *p;
		p++;
	}
	return Result;
}

//static XStringW* XStringW_sprintfBuf;

static void XStringW_transmitSPrintf(const wchar_t* buf, unsigned int nbchar, void* context)
{
//	(*XStringW_sprintfBuf).StrnCat(buf, nbchar);
	((XStringW*)(context))->StrnCat(buf, nbchar);
}

void XStringW::vSWPrintf(const char* format, va_list va)
{
	SetLength(0);

//	XStringW_sprintfBuf = this;

	vwprintf_with_callback(format, va, XStringW_transmitSPrintf, this);
}

// This is an attempt to use _PPrint from IO.c. Problem is : you have to allocate the memory BEFORE calling it.
//void XStringW::vSPrintf(const char* format, va_list va)
//{
//  POOL_PRINT  spc;
//  PRINT_STATE ps;
//
//  ZeroMem(&spc, sizeof (spc));
//  spc.Str = data();
//  SetLength(0);
//  spc.Len = 0;
//  spc.Maxlen = m_size;
//  ZeroMem(&ps, sizeof (ps));
//  ps.Output   = (IN EFI_STATUS (EFIAPI *)(VOID *context, CONST CHAR16 *str))_PoolPrint;
//  ps.Context  = (void*)&spc;
//  ps.fmt.u.pw = format;
//
//  VA_COPY(ps.args, va);
//  _PPrint (&ps);
//  va_end(ps.args);
//}

void XStringW::SWPrintf(const char* format, ...)
{
  va_list     va;

	va_start (va, format);
	vSWPrintf(format, va);
	va_end(va);
}

XStringW XStringW::basename() const
{
	UINTN idx = RIdxOf(LPATH_SEPARATOR);
	if ( idx == MAX_XSIZE ) return NullXStringW;
	return SubString(idx+1, size()-idx-1);
}

XStringW XStringW::dirname() const
{
	UINTN idx = RIdxOf(LPATH_SEPARATOR);
	if ( idx == MAX_XSIZE ) return NullXStringW;
	return SubString(0, idx);
}

XStringW XStringW::SubString(UINTN pos, UINTN count) const
{
	if ( count > size()-pos ) count = size()-pos;
	XStringW ret;
	ret.StrnCat(&(data()[pos]), count);
	return ret;
}

UINTN XStringW::IdxOf(wchar_t aChar, UINTN Pos) const
{
  UINTN Idx;

	for ( Idx=Pos ; Idx<size() ; Idx+=1 ) {
        if ( data()[Idx] == aChar ) return Idx;
	}
	return MAX_XSIZE;
}

UINTN XStringW::IdxOf(const XStringW &S, UINTN Pos) const
{
  UINTN i;
  UINTN Idx;

	if ( size() < S.size() ) return MAX_XSIZE;
	for ( Idx=Pos ; Idx<=size()-S.size() ; Idx+=1 ) {
		i = 0;
	    while( i<S.size()  &&  ( data()[Idx+i] - S._data(0)[i] ) == 0 ) i += 1;
		if ( i == S.size() ) return Idx;
	}
	return MAX_XSIZE;
}

UINTN XStringW::RIdxOf(const wchar_t charToSearch, UINTN Pos) const
{
  UINTN Idx;

	if ( Pos > size() ) Pos = size();
	if ( Pos < 1 ) return MAX_XSIZE;
	for ( Idx=Pos ; Idx-- > 0 ; ) {
		if ( m_data[Idx] == charToSearch ) return Idx;
	}
	return MAX_XSIZE;
}

UINTN XStringW::RIdxOf(const XStringW &S, UINTN Pos) const
{
  UINTN i;
  UINTN Idx;

	if ( S.size() == 0 ) return MAX_XSIZE;
	if ( Pos > size() ) Pos = size();
	if ( Pos < S.size() ) return MAX_XSIZE;
	Pos -= S.size();
	for ( Idx=Pos+1 ; Idx-- > 0 ; ) {
		i = 0;
		while( i<S.size()  &&  data()[Idx+i] == S._data(0)[i] ) i += 1;
		if ( i == S.size() ) return Idx;
	}
	return MAX_XSIZE;
}


bool XStringW::IsDigits() const
{
  const wchar_t *p;

	p = data();
	if ( !*p ) return false;
	for ( ; *p ; p+=1 ) {
		if ( *p < '0' ) return false;
		if ( *p > '9' ) return false;
	}
	return true;
}

bool XStringW::IsDigits(UINTN pos, UINTN count) const
{
  const wchar_t *p;
  const wchar_t *q;

	if ( pos >= size() ) {
		return false;
	}
	if ( pos+count > size() ) {
		return false;
	}
	p = data() + pos;
	q = p + count;
	for ( ; p < q ; p+=1 ) {
		if ( *p < '0' ) return false;
		if ( *p > '9' ) return false;
	}
	return true;
}


void XStringW::RemoveLastEspCtrl()
{
  wchar_t *p;

	if ( size() > 0 ) {
		p = _data(0) + size() - 1;
	#if __WCHAR_MIN__ < 0
		if ( *p >= 0 && *p <= ' ' ) {
	#else
		if ( *p <= ' ' ) {
	#endif
			p -= 1;
	#if __WCHAR_MIN__ < 0
			while ( p>data() && *p >= 0 && *p <= ' ' ) p -= 1;
	#else
			while ( p>data() && *p <= ' ' ) p -= 1;
	#endif
			if ( p>data() ) {
				SetLength( (UINTN)(p-data())+1);
			}else{
	#if __WCHAR_MIN__ < 0
				if ( *p >= 0 && *p <= ' ' ) SetLength(0);
	#else
				if ( *p <= ' ' ) SetLength(0);
	#endif
				else SetLength(1);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
//bool XStringW::ReadFromFILE(FILE *fp)
//{
//  UINTN longueur;
//
//	if ( fread(&longueur, sizeof(longueur), 1, fp) != 1 ) goto fin;
//	if ( longueur > 0  &&  fread(dataWithSizeMin(0, longueur), sizeof(wchar_t), longueur, fp) != 1 ) goto fin;
//	SetLength(longueur);
//	return true;
//  fin:
//	SetNull();
//	return false;
//}
//
//bool XStringW::WriteToFILE(FILE *fp) const
//{
//  UINTN longueur;
//
//	longueur = length();
//	if ( fwrite(&longueur, sizeof(longueur), 1, fp) != 1 ) return false;
//	if ( longueur > 0  &&  fwrite(data(), sizeof(wchar_t), longueur, fp) != 1 ) return false;
//	return true;
//}


//*************************************************************************************************
//
//                                       Operators =
//
//*************************************************************************************************

const XStringW &XStringW::operator =(const XStringW &aString)
{
//TRACE("Operator =const XStringW&\n");
	StrnCpy(aString.data(), aString.size());
	return *this;
}

//
//const XStringW &XStringW::operator =(wchar_t aChar)
//{
////TRACE("Operator =wchar_t \n");
//	StrnCpy(&aChar, 1);
//	return *this;
//}

//const XStringW &XStringW::operator =(const wchar_t *S)
//{
////TRACE("Operator =const wchar_t *\n");
//	if ( S == NULL ) {
//		DBG("operator =(const wchar_t *S) called with NULL\n");
//		panic();
//	}
//	StrCpy(S);
//	return *this;
//}



//*************************************************************************************************
//
//                                       Operators +=
//
//*************************************************************************************************

const XStringW &XStringW::operator +=(wchar_t aChar)
{
//TRACE("Operator +=wchar_t \n");
	StrnCat(&aChar, 1);
	return *this;
}

const XStringW &XStringW::operator +=(const XStringW &aString)
{
//TRACE("Operator +=const XStringW&\n");
	StrnCat(aString.data(), aString.size());
	return *this;
}

const XStringW &XStringW::operator +=(const wchar_t *S)
{
//TRACE("operator +=const wchar_t *\n");
	StrCat(S);
	return *this;
}


//-----------------------------------------------------------------------------
//                                 Functions
//-----------------------------------------------------------------------------

XStringW operator"" _XSW ( const wchar_t* s, size_t len)
{
  XStringW returnValue;
	if ( len > MAX_XSIZE ) len = MAX_XSIZE;
	returnValue.takeValueFrom(s, len);
    return returnValue; // don't do "return returnValue.takeValueFrom(s, len)" because it break the return value optimization.
}

XStringW SWPrintf(const char* format, ...)
{
  va_list     va;
  XStringW str;

  va_start (va, format);
  str.vSWPrintf(format, va);
	va_end(va);

  return str;
}

XStringW SubString(const wchar_t *S, UINTN pos, UINTN count)
{
	if ( wcslen(S)-pos < count ) count = wcslen(S)-pos;
	XStringW ret;
	ret.StrnCpy(S+pos, count);
//	return ( XStringW(S+pos, count) );
	return ret;
}


XStringW CleanCtrl(const XStringW &S)
{
  XStringW ReturnValue;
  UINTN i;

	for ( i=0 ; i<S.size() ; i+=1 ) {
#if __WCHAR_MIN__ < 0
		if ( S.wc_str()[i] >=0  &&  S.wc_str()[i] < ' ' ) ReturnValue += 'x'; /* wchar_t are signed */
#else
		if ( S.wc_str()[i] < ' ' ) ReturnValue += 'x'; /* wchar_t are unsigned */
#endif
		else ReturnValue += S.wc_str()[i];
	}
	return ReturnValue;
}


#endif
