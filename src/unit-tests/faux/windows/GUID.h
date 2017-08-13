#ifndef CS_unittests_faux_windows_guid_h_
#define CS_unittests_faux_windows_guid_h_

typedef struct {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID;

#endif