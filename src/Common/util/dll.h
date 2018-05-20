#ifndef _CS_Common_util_dll_h_
#define _CS_Common_util_dll_h_

#ifndef CS_UNITTESTING
#include <windows.h>
#else
#include "../../unit-tests/faux/windows/WCHAR.h"
#include "../../unit-tests/faux/windows/ARRAYSIZE.h"
#include "../../unit-tests/faux/windows/HMODULE.h"
#include "../../unit-tests/faux/windows/UINT.h"
#include "../../unit-tests/faux/windows/NULL.h"
#endif

#include <string.h>
#include <cwchar>

#define APPLICATION_SPECIFIC_ERROR 0x20000000
#define STATICBUFFER_TOOSMALL (APPLICATION_SPECIFIC_ERROR|0x01)
#define DYNAMICBUFFER_FAILED (APPLICATION_SPECIFIC_ERROR|0x02)
#define DYNAMICBUFFER_TOOSMALL (APPLICATION_SPECIFIC_ERROR|0x03)

HMODULE LoadDLL(LPWSTR dllName, UINT length);
HMODULE GetHandleDLL(LPWSTR dllName, UINT length);

inline HMODULE LoadDLL(const LPWSTR dllName)
{
	if (dllName==NULL) return NULL;
	//https://msdn.microsoft.com/en-us/library/78zh94ax(v=vs.90).aspx
	UINT len=wcslen((const wchar_t *)dllName);
	return LoadDLL(dllName,len);
}

inline HMODULE GetHandleDLL(const LPWSTR dllName)
{
	if (dllName==NULL) return NULL;
	//https://msdn.microsoft.com/en-us/library/ms683199(v=vs.85).aspx
	UINT len=wcslen((const wchar_t *)dllName);
	return GetHandleDLL(dllName,len);
}

#define dll L"dbghelp.dll"
inline HMODULE LoadDLL_dbghelp()
{
	return LoadDLL((const LPWSTR)dll,(UINT)ARRAYSIZE(dll));
}
#undef dll

#define dll L"shell32.dll"
inline HMODULE LoadDLL_shell32()
{
	return GetHandleDLL((const LPWSTR)dll,(UINT)ARRAYSIZE(dll));
}

inline HMODULE GetHandleDLL_shell32()
{
	return LoadDLL((const LPWSTR)dll,(UINT)ARRAYSIZE(dll));
}
#undef dll

inline HMODULE LoadDLL_Riched20()
{
	#define dll L"Riched20.dll"
	return LoadDLL((const LPWSTR)dll,(UINT)ARRAYSIZE(dll));
	#undef dll
}

#define dll L"fmifs.dll"
inline HMODULE LoadDLL_fmifs()
{
	return LoadDLL((const LPWSTR)dll,(UINT)ARRAYSIZE(dll));
}
inline HMODULE GetHandleDLL_fmifs()
{
	return GetHandleDLL((const LPWSTR)dll,(UINT)ARRAYSIZE(dll));
}
#undef dll

inline HMODULE LoadDLL_NETAPI32()
{
	#define dll L"NETAPI32.DLL"
	return LoadDLL((const LPWSTR)dll,(UINT)ARRAYSIZE(dll));
	#undef dll
}

inline HMODULE LoadDLL_srclient()
{
	#define dll L"srclient.dll"
	return LoadDLL((const LPWSTR)dll,(UINT)ARRAYSIZE(dll));
	#undef dll
}

#endif