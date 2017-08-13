#ifndef _CS_Common_util_csstringutil_h_
#define _CS_Common_util_csstringutil_h_

#include "../Tcdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

void UpperCaseCopy (char *lpszDest, const char *lpszSource);
void ToUNICODE (char *lpszText);
void LowerCaseCopy (char *lpszDest, const char *lpszSource);
void LeftPadString (char *szTmp, int len, int targetLen, char filler);
void ToSBCS (LPWSTR lpszText);

#ifdef __cplusplus
}


#include <string>

std::string WideToSingleString (const std::wstring &wideString);
std::wstring SingleStringToWide (const std::string &singleString);
std::wstring Utf8StringToWide (const std::string &utf8String);
std::string WideToUtf8String (const std::wstring &wideString);
std::string ToUpperCase (const std::string &str);
std::string StringToUpperCase (const std::string &str);

#endif


#endif
