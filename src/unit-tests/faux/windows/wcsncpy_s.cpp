#include "wcsncpy_s.h"

#ifdef _MSC_VER
#else
errno_t wcsncpy_s(wchar_t *strDest, size_t numberOfElements, const wchar_t *strSource, size_t count)
{
	return 0;
}
#endif
