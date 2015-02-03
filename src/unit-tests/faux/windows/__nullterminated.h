#ifndef _CS_unittesting_faux_windows___nullterminated_h_
#define _CS_unittesting_faux_windows___nullterminated_h_

#ifdef _MSC_VER
// sal.h is included by MS c++ headers and others, like limits.h
// if we do not use sal.h, then compiler things can go horribly wrong
#include <sal.h>

#else

#ifndef __nullterminated 
#define __nullterminated 
#endif

#endif

#endif
