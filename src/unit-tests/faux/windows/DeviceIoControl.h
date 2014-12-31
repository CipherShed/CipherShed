#ifndef _faux_windows_DeviceIoControl_h_
#define _faux_windows_DeviceIoControl_h_

#include "HANDLE.h"
#include "DWORD.h"
#include "VOID.h"
#include "OVERLAPPED.h"
#include "BOOL.h"

#ifdef __cplusplus
extern "C" {
#endif


BOOL DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);

#ifdef __cplusplus
}
#endif

#endif