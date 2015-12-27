/*  cs_ui.h - CipherShed EFI boot loader
 *
 *
 *
 */

#ifndef _CS_UI_H_
#define _CS_UI_H_

#include <efi.h>
#include <efilib.h>
#include <efibind.h>

#include "cs_common.h"
#include "cs_options.h"
#include "cs_debug.h"
#include "../../Common/Password.h"

#define outId(c,s)	if (!pOptions->flags.silent) { \
					error = uefi_call_wrapper(c->OutputString,2,c,get_string(s));\
					if (EFI_ERROR(error)) { goto ret; }}
#define outStr(c,s)	if (!pOptions->flags.silent) { \
					error = uefi_call_wrapper(c->OutputString,2,c,s);\
					if (EFI_ERROR(error)) { goto ret; }}
#define outErr(c,s)	if (!pOptions->flags.silent) { \
					error = uefi_call_wrapper(c->OutputString,2,c,get_string(CS_STR_ERROR));\
					if (EFI_ERROR(error)) { goto ret; }\
					{ CHAR16 buf[10]; ValueToHex(buf,(UINT64)s);\
					error = uefi_call_wrapper(c->OutputString,2,c,buf);\
					if (EFI_ERROR(error)) { goto ret; }}}

/* available languages for user interface */
enum cs_enum_ui_language {
	CS_LANG_ENG = 0,
	CS_LANG_GER,

	CS_LANG_MAX		/* do not change this entry, it is expected to be the last entry! */
};

/* available strings for user interface to be output in different languages */
enum cs_enum_ui_stings {
	CS_STR_MAIN_TITLE = 1,
	CS_STR_SERVICE_TITLE,
	CS_STR_KBD_CONTROLS,
	CS_STR_KBD_CONTROLS_UNDERLINE,
	CS_STR_ESC,
	CS_STR_SERVICE_MENU,
	CS_STR_ENTER_PASSWD,
	CS_STR_ENTER_OLD_PASSWD,
	CS_STR_ENTER_NEW_PASSWD,
	CS_STR_REENTER_PASSWD,
	CS_STR_WRONG_PASSWD,
	CS_STR_PASSWD_DIFFER,
	CS_STR_SERVICE_OPTIONS,
	CS_STR_SERVICE_OPTIONS_UNDERLINE,
	CS_STR_SERVICE_ESC,
	CS_STR_SERVICE_SELECT,
	CS_STR_SERVICE_DECRYPT_MEDIA,
	CS_STR_SERVICE_ENCRYPT_MEDIA,
	CS_STR_ARE_YOU_SURE,
	CS_STR_SERVICE_CHANGE_PASSWORD,
	CS_STR_ERROR,
};

/* activity that results from the user dialog */
enum cs_enum_user_decision {
	CS_UI_PASSWORD,		/* the default behavior: the user simply typed in his password */
	CS_UI_EXIT_APP,		/* the CS controller needs to be exited (on request by user) */
	CS_UI_REBOOT,		/* the CS controller needs to perform a reboot of the PC (on request by user) */
	CS_UI_SHUTDOWN,		/* the CS controller needs to perform a shutdown of the PC (on request by user) */
	CS_UI_SERVICE_MENU,	/* the user decided to call the service menu */
	CS_UI_MAIN_MENU,	/* the user decided to call the main menu */
	CS_UI_DECRYPT_SYSTEM,	/* permanently decrypt the partition */
	CS_UI_ENCRYPT_SYSTEM,	/* permanently encrypt the partition */
	CS_UI_CHANGE_PASSWD,		/* change user password */
	CS_UI_ESC_PRESSED		/* ESC key pressed */
};

typedef struct
{
	int id;						/* the identifier of the string */
	CHAR16 *value[CS_LANG_MAX];	/* array of strings, sorted by "enum cs_enum_ui_language" */
} uiStrings;

EFI_STATUS reset_input(IN SIMPLE_INPUT_INTERFACE *ConIn);
BOOLEAN check_for_ESC(IN SIMPLE_INPUT_INTERFACE *ConIn);
CHAR16 *get_string(IN const enum cs_enum_ui_stings stringId);
EFI_STATUS ask_for_pwd(IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, IN BOOLEAN showWorng,
		IN const enum cs_enum_ui_stings pwdId);
BOOLEAN check_really_do(IN SIMPLE_INPUT_INTERFACE *ConIn, IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut);
void dump_per_cent(IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, IN INTN value);
EFI_STATUS get_input(IN SIMPLE_INPUT_INTERFACE *ConIn, OPTIONAL SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut,
		OUT void *buffer, IN UINTN buffer_size, IN BOOLEAN stopAtF8,
		IN BOOLEAN dumpString, IN BOOLEAN asciiMode, OUT EFI_INPUT_KEY *key);
EFI_STATUS user_dialog(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,
		IN struct cs_option_data *options,
		OUT enum cs_enum_user_decision *user_decision, OUT Password *passwd);


#endif /* _CS_UI_H_ */
