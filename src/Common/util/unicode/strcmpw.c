#include "strcmpw.h"

#ifndef CS_UNITTESTING
#else
#include "../../../unit-tests/faux/windows/NULL.h"
#endif


int strcmpw(WCHAR* a, WCHAR* b)
{
	if (a==b)
	{
		return 0;
	}
	if (a==NULL)
	{
		return -1;
	}
	if (b==NULL)
	{
		return 1;
	}
	while(1)
	{
		if (*a==*b)
		{
			if (*a==0)
			{
				return 0;
			}
			else
			{
				++a;
				++b;
			}
		}
		else if (*a>*b)
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
}
