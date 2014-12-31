#include "strcpys.h"

#ifndef _MSC_VER

int strcpy_s(char * s1, int s1max, const char * s2)
{
	int res=0;	
	if (!s1) return -1;
	if (!s1max) return -2;
	if (!s2) 
	{
		s1[0]=0;
		return -2;
	}
	char* s1e=s1+s1max;
	while (s1max--)
	{

		if(!(*s1=*s2)) break;
		++s1;
		++s2;
	}
	if (!s1max) *s1e=0;
	return 0;
}

#endif
