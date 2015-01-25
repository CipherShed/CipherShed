#ifndef CS_Common_dialog_errors_h_
#define CS_Common_dialog_errors_h_

#ifndef CS_UNITTESTING
#include <windows.h>
#else
#include "../../unit-tests/faux/windows/DWORD.h"
#include "../../unit-tests/faux/windows/HWND.h"
#include "../../unit-tests/faux/windows/BOOL.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

extern BOOL Silent;

DWORD handleWin32Error ( HWND hwndDlg );
BOOL translateWin32Error ( wchar_t *lpszMsgBuf , int nWSizeOfBuf );
void handleError ( HWND hwndDlg , int code );

void TaskBarIconDisplayBalloonTooltip (HWND hwnd, wchar_t *headline, wchar_t *text, BOOL warning);
void InfoBalloon (const char *headingStringId, const char *textStringId);
void InfoBalloonDirect (const wchar_t *headingString, const wchar_t *textString);
void WarningBalloon (const char *headingStringId, const char *textStringId);
void WarningBalloonDirect (const wchar_t *headingString, const wchar_t *textString);
int Info (char *stringId);
int InfoTopMost (char *stringId);
int InfoDirect (const wchar_t *msg);
int Warning (char *stringId);
int WarningTopMost (char *stringId);
int WarningDirect (const wchar_t *warnMsg);
int Error (const char *stringId);
int ErrorDirect (const wchar_t *errMsg);
int ErrorDirectTitle (const wchar_t* errMsg, const wchar_t* title);
int ErrorTopMost (char *stringId);
int AskYesNo (char *stringId);
int AskYesNoString (const wchar_t *str);
int AskYesNoTopmost (char *stringId);
int AskNoYes (char *stringId);
int AskOkCancel (char *stringId);
int AskWarnYesNo (char *stringId);
int AskWarnYesNoTitle (char* stringId, wchar_t* title);
int AskWarnYesNoString (const wchar_t *string);
int AskWarnYesNoTopmost (char *stringId);
int AskWarnYesNoStringTopmost (const wchar_t *string);
int AskWarnNoYes (char *stringId);
int AskWarnNoYesString (const wchar_t *string);
int AskWarnNoYesTopmost (char *stringId);
int AskWarnOkCancel (char *stringId);
int AskWarnCancelOk (char *stringId);
int AskErrYesNo (char *stringId);
int AskErrNoYes (char *stringId);

void InconsistencyResolved (char *msg);
void ReportUnexpectedState (char *techInfo);


#ifdef __cplusplus
}

#include <string>


std::wstring GetWrongPasswordErrorMessage (HWND hwndDlg);

#endif

#endif