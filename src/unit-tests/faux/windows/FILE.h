#ifndef _faux_windows_file_h_
#define _faux_windows_file_h_

#ifndef _FILE_DEFINED
#ifdef _MSC_VER
struct _iobuf {
        char *_ptr;
        int   _cnt;
        char *_base;
        int   _flag;
        int   _file;
        int   _charbuf;
        int   _bufsiz;
        char *_tmpfname;
        };
typedef struct _iobuf FILE;
#else
#ifdef __CYGWIN__ 
#include <sys/reent.h>
typedef __FILE FILE;
#else
#include <stdio.h>
#endif
#endif
#define _FILE_DEFINED
#endif

#endif