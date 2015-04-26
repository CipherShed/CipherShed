/*
 *
 *
 *
 */

#include "cs_options.h"
#include "cs_ui.h"
#include "cs_controller.h"


#define entry_check(s)		((size1 == StrLen(WIDEN(s)) && (strncmpa(word1, (CHAR8*)s, size1) == 0)))

static struct cs_option_data *pOptions = NULL;	/* pointer to options from option file
 	 	 	 	 	 	 	 	 	 	 	 	   (part of controller context) */

/* additional supported languages need to be inserted here together with an ID for the options file,
 * Attention: these strings must be UPPERCASE !! */
static struct cs_language_name lang_strings[] =
{
		{ CS_LANG_ENG, "ENG" },
		{ CS_LANG_GER, "GER" },
		/**********************************************************************/
		{ CS_LANG_MAX, "" }		/* marks the end of the array, do not change! */
};

/*
 *	\brief	function to separate lines in the given buffer
 *
 *	The function scans for line break characters and replaces this with 0 in order to separate
 *	the lines in terms of ASCII strings. Also, the line size and the position of the next line
 *	is returned.
 *
 *	\param	buffer	input buffer containing the file content
 *	\param	buffer_size	size of the buffer
 *	\param	line_size	pointer to the returned size of the line
 *
 *	\return		position in buffer of the begin of the next line (0 in case of EOF)
 */
static UINTN _get_next_line(IN CHAR8 *buffer, IN UINTN buffer_size, OUT UINTN *line_size) {
	UINTN i = 0;
	UINTN next;
	int success = 0;

    ASSERT(buffer != NULL);
    ASSERT(line_size != NULL);

	*line_size = 0;

	while (buffer_size > 0) {
		if ((buffer[i] != '\r') && (buffer[i] != '\n'))  {
			i++;
			buffer_size--;
			(*line_size)++;
		} else {
			success = 1;
			break;
		}
	}
	if (success) {
		buffer[i] = 0;
		if ((buffer_size > 0) && (buffer[i] == '\r') && (buffer[i + 1] == '\n')) {
			i++;
			buffer_size--;
			buffer[i] = 0;
		}
		next = i + 1;	/* index of next line */
	} else {
		next = 0;		/* end of buffer */
	}

	return next;
}

/*
 * 	\brief	strip redundant characters from a single line
 *
 * 	The function removes heading and tailing whitespace from a given line. Comment lines
 * 	(starting with "#") will be removed: This means that the returned line size is 0.
 *
 *	\param	line	the ASCII representation of the line to parse
 * 	\remaining_characters	the size of the line (remaining characters until EOL)
 *
 * 	\return		resulting line size
 */
static int _strip_line(IN OUT CHAR8 *line, IN UINTN remaining_characters) {

	UINTN i, index = 0;

    ASSERT(line != NULL);

	/* strip leading whitespace */
	while ((remaining_characters > 0) && ((line[0] == ' ') || (line[0] == '\t'))) {
		for (i = 0; i < remaining_characters; i++)
			line[i] = line[i + 1];
		remaining_characters--;
	}

	/* strip tailing whitespace */
	while ((remaining_characters > 0) &&
			((line[remaining_characters - 1] == ' ') || (line[remaining_characters - 1] == '\t'))) {
		line[remaining_characters - 1] = 0;
		remaining_characters--;
	}

	/* ignore comment lines starting with '#' */
	if ((remaining_characters > 0) && (line[0] == '#')) /* ignore comment lines */
	{
		return 0;
	}

	/* search the first '=' character */
	while ((remaining_characters > 0) && (line[index] != '=')) {
		remaining_characters--;
		index++;
	}

	/* strip whitespace directly before '=' */
	if (line[index] == '=') {

		/* strip whitespace directly before '=' */
		while ((index > 0) && ((line[index - 1] == ' ') || (line[index - 1] == '\t'))) {
			UINTN j;
			for (j = index; j <= index + remaining_characters; j++)
				line[j - 1] = line[j];
			index--;
		}

		/* strip whitespace directly after '=' */
		while ((remaining_characters > 0) && ((line[index + 1] == ' ') || (line[index + 1] == '\t'))) {
			for (i = index + 1; i <= index + remaining_characters; i++)
				line[i] = line[i + 1];
			remaining_characters--;
		}
	}

	return index + remaining_characters;
}

/*
 *	\brief	convert an ASCII string to upper case
 *
 *	The given string is replaced by the string in upper case.
 *
 *	\param	string	ASCII string to convert
 *	\param	size	size of the ASCII string
 */
static void _ascii_to_upper(IN OUT CHAR8 *string, IN const UINTN size) {
	UINTN i;

    ASSERT(string != NULL);

    for (i = 0; i < size; i++)
        if (*(string + i) >= 'a' && *(string + i) <= 'z')
            *(string + i) -= ('a' - 'A');
}

/*
 * \brief	convert an ASCII string to a number
 *
 * In case of a heading "0x", the number is taken as hex number, otherwise decimal.
 *
 *	\param	word	ASCII string containing the number to convert
 *	\param	size	size of the ASCII string
 *
 *	\return			number value of the string
 */
static UINTN _ascii_to_number(IN const CHAR8 *word, IN const UINTN size) {

	CHAR16 *string;
	UINTN result;

    ASSERT(word != NULL);

	string = AllocatePool(4 * size);

	if (string) {
		SPrint(string, 4 * size, L"%a", word);

		if ((StrLen(string) > 2) && ((StrnCmp(string, L"0x", 2) == 0) || (StrnCmp(string, L"0X", 2) == 0))) {
			result = xtoi(&string[2]);
		} else {
			result = Atoi(string);
		}
	}
	FreePool(string);

	return result;
}

/*
 * \brief	function to parse a single line in the options file
 *
 * As result of (successful) parsing, the corresponding option/setting of the (global) context
 * is overwritten. In case that the parsing fails, the option/setting remains unchanged.
 *
 *	\param	line	single line to be parsed
 *	\param	size	size of the line
 */
static void parse_line(IN CHAR8 *line, IN UINTN size) {

	const CHAR8 *word1 = line;
	UINTN size1 = size;
	CHAR8 *word2 = line;
	UINTN size2 = 0;
	UINTN i;

    ASSERT(line != NULL);
    ASSERT(pOptions != NULL);

	_ascii_to_upper(line, size);

	/* try to find a '=' as separator between word1 and word2 */
	for (i = 0; i < size; i++)
		if (line[i] == '=') {
			size1 = i;
			word2 = &line[i + 1];
			size2 = size - i - 1;
		}

	CS_DEBUG((D_INFO, L"Option line: %a, word1: %a (%x), word2: %a (%x)\n", line, word1, size1, word2, size2));

	if (entry_check("DEBUG")) {
		EFIDebug = _ascii_to_number(word2, size2);
		CS_DEBUG((D_INFO, L"Option: Debug -> 0x%x\n", EFIDebug));
	} else
	if (entry_check("DRIVER_DEBUG")) {
		pOptions->driverdebug = _ascii_to_number(word2, size2);
		CS_DEBUG((D_INFO, L"Option: Driver Debug -> 0x%x\n", EFIDebug));
	} else
	if (entry_check("NOSILENT")) {
		pOptions->flags.silent = 0;
		CS_DEBUG((D_INFO, L"Option: NOSILENT\n"));
	} else
	if (entry_check("LANG")) {
		int i;
		for (i = 0; lang_strings[i].id < CS_LANG_MAX; i++) {
			if ((size2 == strlena((CHAR8 *)lang_strings[i].value))
					&& (strncmpa(word2, (CHAR8 *)lang_strings[i].value, size2) == 0)) {
				pOptions->language = lang_strings[i].id;
				CS_DEBUG((D_INFO, L"Option: language -> 0x%x\n", pOptions->language));
				break;
			}
		}
	} else
	if (entry_check("ENABLE_SERVICE_MENU")) {
		pOptions->flags.enable_service_menu = 1;
		CS_DEBUG((D_INFO, L"Option: ENABLE_SERVICE_MENU\n"));
	}	else
	if (entry_check("SHOW_SERVICE_MENU")) {
		pOptions->flags.show_service_menu = 1;
		CS_DEBUG((D_INFO, L"Option: SHOW_SERVICE_MENU\n"));
	}
	if (entry_check("ENABLE_MEDIA_DECRYPTION")) {
		pOptions->flags.enable_media_decryption = 1;
		CS_DEBUG((D_INFO, L"Option: ENABLE_MEDIA_DECRYPTION\n"));
	}
	if (entry_check("ENABLE_MEDIA_ENCRYPTION")) {
		pOptions->flags.enable_media_encryption = 1;
		CS_DEBUG((D_INFO, L"Option: ENABLE_MEDIA_ENCRYPTION\n"));
	}
	if (entry_check("ENABLE_PASSWORD_CHANGE")) {
		pOptions->flags.enable_password_change = 1;
		CS_DEBUG((D_INFO, L"Option: ENABLE_PASSWORD_CHANGE\n"));
	}
	if (entry_check("ENABLE_PASSWORD_ASTERISK")) {
		pOptions->flags.enable_password_asterisk = 1;
		CS_DEBUG((D_INFO, L"Option: ENABLE_PASSWORD_ASTERISK\n"));
	}

	/* more options might be:
	 * - encrypt (automatically trigger the encryption of the media), set by the OS installer
	 * - decrypt (automatically trigger the decryption of the media), set by the OS deinstaller
	 * - show service menu (automatically show the service menu)  */

}

/*
 * 	\brief	main function to parse the content of the options file.
 *
 * 	The function does not fail: in case that (part of) the content cannot be parsed,
 * 	the corresponding option remains with its default value
 *
 *	\param	buffer	buffer containing the file content
 *	\param	size	size of the buffer
 */
static void parse_settings_buffer(IN OUT CHAR8 *buffer, IN UINTN size) {
	UINTN next, index = 0;
	UINTN line_size;

    ASSERT(buffer != NULL);

	do {
		next = _get_next_line(&buffer[index], size, &line_size);
		line_size = _strip_line(&buffer[index], line_size);
		if (line_size)
			parse_line(&buffer[index], line_size);
		index += next;
		size -= next;
	} while (next > 0);
}

/*
 *	\brief	main function to load the options from a defined options file
 *
 *	This function loads and parses the settings from the options file. The parsed settings
 *	will overwrite the default values of the (global) system context.
 *
 *	\param	root_dir	handle to the opened root directory
 *	\param	current_dir	string containing the directory name of the executed application
 *
 *	\return		the success state of the function
 */
EFI_STATUS load_settings(IN EFI_FILE *root_dir, IN CHAR16 *current_dir, IN struct cs_option_data *options) {
    EFI_STATUS error = EFI_SUCCESS;
    CHAR16 *buf;
    UINTN buflen;

    ASSERT(root_dir != NULL);
    ASSERT(current_dir != NULL);
    ASSERT(options != NULL);

    pOptions = options;	/* initialize global variable */

    buflen = StrLen(current_dir) + StrLen((const CHAR16 *)CS_FILENAME_OPTIONS) + 1;
    buf = AllocatePool(buflen);
    if (buf) {
    	UINTN size_settings_file;
    	CHAR8 *content_settings_file;

    	StrCpy(buf, current_dir);
    	StrCat(buf, (const CHAR16 *)CS_FILENAME_OPTIONS);
       	CS_DEBUG((D_INFO, L"Settings Filename is: %s\n", buf));
       	//Print(L"Settings Filename is: %s\n", buf);

    	size_settings_file = read_file(root_dir, buf, &content_settings_file);
    	if (size_settings_file) {
    		parse_settings_buffer(content_settings_file, size_settings_file);
    		FreePool(content_settings_file);
    	}
    	FreePool(buf);
    }
    else {
		CS_DEBUG((D_ERROR, L"Unable to allocate filename buffer (0x%x byte)\n", buflen));
		error = EFI_BUFFER_TOO_SMALL;
    }

	return error;
}
