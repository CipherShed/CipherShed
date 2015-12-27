/*  cs_options.h - CipherShed EFI boot loader
 *
 *
 *
 */

#ifndef _CS_OPTIONS_H_
#define _CS_OPTIONS_H_

#include "cs_common.h"

#define CS_FILENAME_OPTIONS			L"settings"	/* filename for the settings */

/* available for the options file: language supported by the loader */
struct cs_language_name
{
	UINTN id;						/* the identifier of the language */
	char *value;				/* ASCII string as expected in options file for selection of current language */
};

/* options taken from options file (see parse_line()) */
struct cs_option_data {
	UINTN driverdebug;		/* debugging level for the driver, see efidebug.h */
	UINTN language;			/* language setting of the loader, see lang_strings[] */
	struct {
		UINT32 silent:1;	/* if set then user output (main menu/service menu) is shown */
		UINT32 enable_service_menu:1;	/* service menu can be opened */
		UINT32 show_service_menu:1;		/* link to service menu is shown in main menu */
		UINT32 enable_media_decryption:1;	/* media decryption can be performed */
		UINT32 enable_media_encryption:1;	/* media encryption can be performed */
		UINT32 enable_password_change:1;	/* password change can be performed */
		UINT32 enable_password_asterisk:1;	/* show asterisks when typing in the password */
	} flags;
};


EFI_STATUS load_settings(IN EFI_FILE *root_dir, IN CHAR16 *current_dir, IN struct cs_option_data *options);


#endif /* _CS_OPTIONS_H_ */
