//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//
//                                           STRINGS
//
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

#if !defined(__XSTRINGWS_CPP__)
#define __XSTRINGWS_CPP__

#include "XToolsCommon.h"
#include "XStringWArray.h"


const XStringWArray NullXStrings;


XStringWArray::XStringWArray() : XStringWArraySuper()
{
}

//-------------------------------------------------------------------------------------------------
//
//-------------------------------------------------------------------------------------------------
void XStringWArray::AddStrings(const wchar_t *Val1, ...)
{
  va_list va;
  const wchar_t *p;

	{
		XStringW* newS = new XStringW;
		newS->takeValueFrom(Val1);
		AddReference(newS, true);
	}
	va_start(va, Val1);
	p = VA_ARG(va, const wchar_t *);
	while ( p != nullptr ) {
		XStringW* newS = new XStringW;
		newS->takeValueFrom(Val1);
		AddReference(newS, true);
		p = VA_ARG(va, const wchar_t *);
	}
	va_end(va);
}

XStringW XStringWArray::ConcatAll(const XStringW& Separator, const XStringW& Prefix, const XStringW& Suffix) const
{
  xsize i;
  XStringW s;

	if ( size() > 0 ) {
		s = Prefix;
		s += ElementAt(0);
		for ( i=1 ; i<size() ; i+=1 ) {
			s += Separator;
			s += ElementAt(i);
		}
		s += Suffix;
	}
	return s;
}

bool XStringWArray::Equal(const XStringWArray &aStrings) const
{
  xsize ui;

	if ( size() != aStrings.size() ) return false;
	for ( ui=0 ; ui<size() ; ui+=1 ) {
		if ( ElementAt(ui) != aStrings[ui] ) return false;
	}
	return true;
}

bool XStringWArray::Same(const XStringWArray &aStrings) const
{
  xsize i;

	for ( i=0 ; i<size() ; i+=1 ) {
		if ( !aStrings.Contains(ElementAt(i)) ) return false;
	}
	for ( i=0 ; i<aStrings.size() ; i+=1 ) {
		if ( !Contains(aStrings.ElementAt(i)) ) return false;
	}
	return true;
}

bool XStringWArray::Contains(const XStringW &S) const
{
  xsize i;

	for ( i=0 ; i<size() ; i+=1 ) {
		if ( ElementAt(i) == S ) return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
// Add
//-------------------------------------------------------------------------------------------------
void XStringWArray::Add(const XStringWArray &aStrings)
{
  xsize i;

	for ( i=0 ; i<aStrings.size() ; i+=1 ) {
		AddCopy(aStrings[i]);
	}
}

void XStringWArray::AddID(const XStringW &aString)
{
	if ( !Contains(aString) ) AddCopy(aString);
}

void XStringWArray::AddID(const XStringWArray &aStrings)
{
  xsize i;

	for ( i=0 ; i<aStrings.size() ; i+=1 ) {
		if ( !Contains(aStrings[i]) ) AddCopy(aStrings[i]);
	}
}


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Divers
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

XStringWArray Split(const XStringW &S, const XStringW &Separator)
{
  XStringWArray Ss;
  xsize idxB, idxE;

	idxB = 0;
	idxE = S.IdxOf(Separator, idxB);
	while ( idxE != MAX_XSIZE ) {
		Ss.AddCopy(SubString(S.wc_str(), idxB, idxE-idxB));
		idxB = idxE + Separator.size();
		idxE = S.IdxOf(Separator, idxB);
	}
	if ( idxB < S.size() ) Ss.AddCopy(SubString(S.wc_str(), idxB, S.size()-idxB));
	return Ss;
}



#endif
