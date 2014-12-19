/**
* (c) 2014 CipherShed. This is licensed under CipherShed license.
*
*/

#ifndef _snprintf_h_
#define _snprintf_h_

#include <stdio.h> 

#ifdef _MSC_VER
/*
* http://msdn.microsoft.com/en-us/library/ce3zzk1k.aspx or the solution 
* presented in http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
*/

#define snprintf sprintf_s

#endif

#define ARRAY_LENGTH(x)  (sizeof(x) / sizeof(x[0]))


#endif