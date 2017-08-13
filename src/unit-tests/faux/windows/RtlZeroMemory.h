#ifndef CS_unittests_faux_windows_RtlZeroMemory_h_
#define CS_unittests_faux_windows_RtlZeroMemory_h_

#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#endif
