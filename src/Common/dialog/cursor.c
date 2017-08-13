#include "cursor.h"

#ifdef CS_UNITTESTING
#include "../../unit-tests/faux/windows/NULL.h"
#include "../../unit-tests/faux/windows/LoadCursor.h"
#include "../../unit-tests/faux/windows/SetCursor.h"
#include "../../unit-tests/faux/windows/IDC_x.h"
#endif


HCURSOR hCursor = NULL;

/**
set the cursor to the "waiting", loading the cursor from the resources.
@TODO LoadCursor is depricated per https://msdn.microsoft.com/en-us/library/windows/desktop/ms648391(v=vs.85).aspx
@TODO why are we using "our" cursor resource, shouldn't the OS provide this detail?
*/
void
WaitCursor ()
{
	static HCURSOR hcWait;
	if (hcWait == NULL)
		hcWait = LoadCursor (NULL, IDC_WAIT);
	SetCursor (hcWait);
	hCursor = hcWait;
}

void
NormalCursor ()
{
	static HCURSOR hcArrow;
	if (hcArrow == NULL)
		hcArrow = LoadCursor (NULL, IDC_ARROW);
	SetCursor (hcArrow);
	hCursor = NULL;
}

void
ArrowWaitCursor ()
{
	static HCURSOR hcArrowWait;
	if (hcArrowWait == NULL)
		hcArrowWait = LoadCursor (NULL, IDC_APPSTARTING);
	SetCursor (hcArrowWait);
	hCursor = hcArrowWait;
}

void HandCursor ()
{
	static HCURSOR hcHand;
	if (hcHand == NULL)
		hcHand = LoadCursor (NULL, IDC_HAND);
	SetCursor (hcHand);
	hCursor = hcHand;
}

