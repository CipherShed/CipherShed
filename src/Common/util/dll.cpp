#include "dll.h"

#define LoadDLL_stack_buf_size	MAX_PATH
#define BACKSLASH_STRING		L"\\"

#include <stdlib.h>
#include <string.h>

#ifndef CS_UNITTESTING
#else
#include "../../unit-tests/faux/windows/SetLastError.h"
#include "../../unit-tests/faux/windows/GetSystemDirectory.h"
#include "../../unit-tests/faux/windows/LoadLibraryEx.h"
#include "../../unit-tests/faux/windows/GetModuleHandle.h"
#include "../../unit-tests/faux/windows/MAX_PATH.h"
#include "../../unit-tests/faux/windows/errno_t.h"
#include "../../unit-tests/faux/windows/wcsncpy_s.h"
#endif



inline errno_t append(LPWSTR buf,UINT buf_size, UINT &ofs, LPWSTR str, UINT str_len)
{
	//https://msdn.microsoft.com/en-us/library/5dae5d43(v=vs.110).aspx
	errno_t err=wcsncpy_s((wchar_t *)&buf[ofs],buf_size-ofs,(const wchar_t *)str,str_len);
	if (!err) ofs+=str_len;
	return err;
}

HMODULE LoadDLLdbuffer(UINT systemPrefixLength, LPWSTR dllName, UINT length)
{
	//printf("LoadDLLdbuffer=>\n syspreflen=%d\n dllName=%S\n length=%d\n",systemPrefixLength, dllName, length);
	// code not implemented.
	
	UINT blen=systemPrefixLength+1+length+1;
	WCHAR* buf=(WCHAR*)malloc(sizeof(WCHAR)*(blen));
	if (!buf)
	{
		SetLastError(DYNAMICBUFFER_FAILED);	
		return NULL;
	}
	UINT len=GetSystemDirectoryW(buf,blen);

	errno_t err;
	if (buf[len]!=BACKSLASH_STRING[0])		
	{
		err=append(buf,blen,len,(LPWSTR)BACKSLASH_STRING,wcslen(BACKSLASH_STRING));
		if (err)
		{
			free(buf);
			SetLastError(DYNAMICBUFFER_TOOSMALL);
			return NULL;
		}
	}

	err=append(buf,blen,len,dllName,length);
	if (err)
	{
		free(buf);
		SetLastError(DYNAMICBUFFER_TOOSMALL);
		return NULL;
	}

	DWORD   dwFlags=0;
	//https://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
	HMODULE res;
	res=LoadLibraryExW(buf, NULL, dwFlags);

	free(buf);

	return res;
}

HMODULE GetHandleDLLdbuffer(UINT systemPrefixLength, LPWSTR dllName, UINT length)
{
	//printf("LoadDLLdbuffer=>\n syspreflen=%d\n dllName=%S\n length=%d\n",systemPrefixLength, dllName, length);
	// code not implemented.
	
	UINT blen=systemPrefixLength+1+length+1;
	WCHAR* buf=(WCHAR*)malloc(sizeof(WCHAR)*(blen));
	if (!buf)
	{
		SetLastError(DYNAMICBUFFER_FAILED);	
		return NULL;
	}
	UINT len=GetSystemDirectoryW(buf,blen);

	errno_t err;
	if (buf[len]!=BACKSLASH_STRING[0])		
	{
		err=append(buf,blen,len,(LPWSTR)BACKSLASH_STRING,wcslen(BACKSLASH_STRING));
		if (err)
		{
			free(buf);
			SetLastError(DYNAMICBUFFER_TOOSMALL);
			return NULL;
		}
	}

	err=append(buf,blen,len,dllName,length);
	if (err)
	{
		free(buf);
		SetLastError(DYNAMICBUFFER_TOOSMALL);
		return NULL;
	}

	//https://msdn.microsoft.com/en-us/library/ms683199(v=vs.85).aspx
	HMODULE res;
	res=GetModuleHandleW(buf);

	free(buf);

	return res;
}

HMODULE LoadDLL(LPWSTR dllName, UINT length)
{
	const int minlen=LoadDLL_stack_buf_size-1-length-1;

	WCHAR stackBuf[LoadDLL_stack_buf_size];
	stackBuf[LoadDLL_stack_buf_size-1]=0;

	// windows 2000 support, not supported in NT4
	//https://msdn.microsoft.com/en-us/library/windows/desktop/ms724373(v=vs.85).aspx
	UINT len=GetSystemDirectoryW(stackBuf,LoadDLL_stack_buf_size);

	// in case UINT is redefined as signed...
	if (len<=0) 
	{
		// the caller may GetLastError ...
		return NULL;
	}
	else if (minlen<=0 || minlen<(int)len)
	{
		//use dynamic buffer
		return LoadDLLdbuffer(len,dllName,length);
	}

	errno_t err;
	if (stackBuf[len]!=BACKSLASH_STRING[0])		
	{
		//https://msdn.microsoft.com/en-us/library/5dae5d43(v=vs.110).aspx
		err=append(stackBuf,LoadDLL_stack_buf_size,len,(LPWSTR)BACKSLASH_STRING,wcslen(BACKSLASH_STRING));
		if (err)
		{
			SetLastError(STATICBUFFER_TOOSMALL);
			return NULL;
		}
	}

	err=append(stackBuf,LoadDLL_stack_buf_size,len,dllName,length);
	if (err)
	{
		SetLastError(STATICBUFFER_TOOSMALL);
		return NULL;
	}

	//https://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
	return LoadLibraryExW(stackBuf, NULL, 0);
}

HMODULE GetHandleDLL(LPWSTR dllName, UINT length)
{
	const int minlen=LoadDLL_stack_buf_size-1-length-1;

	WCHAR stackBuf[LoadDLL_stack_buf_size];
	stackBuf[LoadDLL_stack_buf_size-1]=0;

	// windows 2000 support, not supported in NT4
	//https://msdn.microsoft.com/en-us/library/windows/desktop/ms724373(v=vs.85).aspx
	UINT len=GetSystemDirectoryW(stackBuf,LoadDLL_stack_buf_size);

	// in case UINT is redefined as signed...
	if (len<=0) 
	{
		// the caller may GetLastError ...
		return NULL;
	}
	else if (minlen<=0 || minlen<(int)len)
	{
		//use dynamic buffer
		return GetHandleDLLdbuffer(len,dllName,length);
	}

	errno_t err;
	if (stackBuf[len]!=BACKSLASH_STRING[0])		
	{
		//https://msdn.microsoft.com/en-us/library/5dae5d43(v=vs.110).aspx
		err=append(stackBuf,LoadDLL_stack_buf_size,len,(LPWSTR)BACKSLASH_STRING,wcslen(BACKSLASH_STRING));
		if (err)
		{
			SetLastError(STATICBUFFER_TOOSMALL);
			return NULL;
		}
	}

	err=append(stackBuf,LoadDLL_stack_buf_size,len,dllName,length);
	if (err)
	{
		SetLastError(STATICBUFFER_TOOSMALL);
		return NULL;
	}

	//https://msdn.microsoft.com/en-us/library/ms683199(v=vs.85).aspx
	return GetModuleHandleW(stackBuf);
}


