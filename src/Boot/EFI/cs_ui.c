/*  cs_ui.c - CipherShed EFI boot loader
 *  implementation of the user interface of the EFI loader
 *
 *
 *
 */

#include "cs_common.h"
#include "cs_ui.h"
#include "cs_service.h"
#include "cs_controller.h"

static INT32 oldMode;			/* initial mode of console output */
static INT32 currentMode;		/* current mode of console output */
static UINTN columns, rows;     /* dimensions of the current console output mode */

static struct cs_option_data *pOptions = NULL;	/* pointer to options from option file
 	 	 	 	 	 	 	 	 	 	 	 	   (part of controller context) */

/* language dependent strings fur user interface: this need to be changed in case of additional
 * languages or additional strings to be output */
static uiStrings ui_strings[] =
{
	/* string ID, array of strings (sorted by language) */

	{ CS_STR_MAIN_TITLE,
			{ /* CS_LANG_ENG */ L" " CS_LOADER_NAME L" Boot Loader " WIDEN(STRINGIFY(CS_LOADER_VERSION)),
			  /* CS_LANG_GER */ L" " CS_LOADER_NAME L" Boot Loader " WIDEN(STRINGIFY(CS_LOADER_VERSION)) },
	},
	{ CS_STR_SERVICE_TITLE,
			{ /* CS_LANG_ENG */ L" " CS_LOADER_NAME L" Service Menu ",
			  /* CS_LANG_GER */ L" " CS_LOADER_NAME L" Service Menü " },
	},
	{ CS_STR_KBD_CONTROLS,
			{ /* CS_LANG_ENG */ L"Keyboard Controls:",
			  /* CS_LANG_GER */ L"Tastensteuerung:" },
	},
	{ CS_STR_KBD_CONTROLS_UNDERLINE,
			{ /* CS_LANG_ENG */ L"------------------",
			  /* CS_LANG_GER */ L"----------------" },
	},
	{ CS_STR_ESC,
			{ /* CS_LANG_ENG */ L"[ESC]  Skip Authentication (Boot Manager)",
			  /* CS_LANG_GER */ L"[ESC]  Anmeldung überspringen (Boot Manager)" },
	},
	{ CS_STR_SERVICE_MENU,
			{ /* CS_LANG_ENG */ L"[F8]   Enter Service Menu",
			  /* CS_LANG_GER */ L"[F8]   Aufruf Service Menü" },
	},
	{ CS_STR_ENTER_PASSWD,
			{ /* CS_LANG_ENG */ L"Enter password: ",
			  /* CS_LANG_GER */ L"Eingabe Paßwort: " },
	},
	{ CS_STR_ENTER_OLD_PASSWD,
			{ /* CS_LANG_ENG */ L"Enter old password: ",
			  /* CS_LANG_GER */ L"Eingabe altes Paßwort: " },
	},
	{ CS_STR_ENTER_NEW_PASSWD,
			{ /* CS_LANG_ENG */ L"Enter new password: ",
			  /* CS_LANG_GER */ L"Eingabe neues Paßwort: " },
	},
	{ CS_STR_REENTER_PASSWD,
			{ /* CS_LANG_ENG */ L"Repeat new password: ",
			  /* CS_LANG_GER */ L"Wiederholung neues Paßwort: " },
	},
	{ CS_STR_WRONG_PASSWD,
			{ /* CS_LANG_ENG */ L"Password wrong!",
			  /* CS_LANG_GER */ L"Paßwort falsch!" },
	},
	{ CS_STR_PASSWD_DIFFER,
			{ /* CS_LANG_ENG */ L"Mismatch!",
			  /* CS_LANG_GER */ L"Paßworte sind unterschiedlich!" },
	},
	{ CS_STR_SERVICE_OPTIONS,
			{ /* CS_LANG_ENG */ L"Available Service Options:",
			  /* CS_LANG_GER */ L"mögliche Service-Optionen:" },
	},
	{ CS_STR_SERVICE_OPTIONS_UNDERLINE,
			{ /* CS_LANG_ENG */ L"--------------------------",
			  /* CS_LANG_GER */ L"--------------------------" },
	},
	{ CS_STR_SERVICE_ESC,
			{ /* CS_LANG_ENG */ L"[ESC]  Cancel",
			  /* CS_LANG_GER */ L"[ESC]  Abbruch" },
	},
	{ CS_STR_SERVICE_SELECT,
			{ /* CS_LANG_ENG */ L"To select, press the corresponding key: ",
			  /* CS_LANG_GER */ L"Zur Auswahl bitte Taste drücken: " },
	},
	{ CS_STR_SERVICE_DECRYPT_MEDIA,
			{ /* CS_LANG_ENG */ L"[1]    Permanently decrypt system partition",
			  /* CS_LANG_GER */ L"[1]    Entschlüsseln der Systempartition" },
	},
	{ CS_STR_SERVICE_ENCRYPT_MEDIA,
			{ /* CS_LANG_ENG */ L"[2]    Permanently encrypt system partition",
			  /* CS_LANG_GER */ L"[2]    Verschlüsseln der Systempartition" },
	},
	{ CS_STR_ARE_YOU_SURE,
			{ /* CS_LANG_ENG */ L"Are you sure? (y/N)",
			  /* CS_LANG_GER */ L"Sind Sie sicher? (j/N)" },
	},
	{ CS_STR_SERVICE_CHANGE_PASSWORD,
			{ /* CS_LANG_ENG */ L"[3]    Change password",
			  /* CS_LANG_GER */ L"[3]    Paßwort ändern" },
	},
	{ CS_STR_ERROR,
			{ /* CS_LANG_ENG */ L"Error ",
			  /* CS_LANG_GER */ L"Fehler " },
	},

	/******************************************************************************/
	{ 0, 	/* marks the end of the array, do not change this value! */
			{ /* CS_LANG_ENG */ L"",
			  /* CS_LANG_GER */ L"" }
	}
};

/* used to store (old) cursor positions */
struct cs_output_context {
	INT32 row, column;
};

/*
 *  \brief	store current cursor position
 *
 *	store current cursor position and return it
 *
 *	\param	ConOut	output console interface
 *
 *	\return		the structure containing the cursor position
 */
static struct cs_output_context store_output_context(IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut) {
	struct cs_output_context context;

	ASSERT(ConOut != NULL);

	context.column = ConOut->Mode->CursorColumn;
	context.row = ConOut->Mode->CursorRow;

	return context;
}

/*
 *  \brief	restore current cursor position
 *
 *	Restore current cursor position to the given value. Optionally the space between
 *	the current position and the stored position can be cleaned (only if the stored
 *	position is above/before the actual position).
 *
 *	\param	ConOut		output console interface
 *	\param	pContext	the stored position to be activated
 *	\param	cleanArea	flag indicating whether the area between the stored and the
 *						actual position shall be cleaned
 *
 *	\return		the success state of the function
 */
static EFI_STATUS restore_output_context(IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut,
		IN struct cs_output_context *oldContext, IN BOOLEAN cleanArea) {

	ASSERT(ConOut != NULL);
	ASSERT(oldContext != NULL);

	if (pOptions->flags.silent)
		return EFI_SUCCESS;

	if ((oldContext->column == ConOut->Mode->CursorColumn) && (oldContext->row == ConOut->Mode->CursorRow)) {
		return EFI_SUCCESS;	/* nothing to do */
	}

	if (cleanArea) {
		struct cs_output_context context;
		context.column = ConOut->Mode->CursorColumn;
		context.row = ConOut->Mode->CursorRow;

		if ((oldContext->row < context.row)
				|| ((oldContext->row == context.row) && (oldContext->column < context.column))) {
			INT32 row = oldContext->row;
			INT32 col = oldContext->column;

			uefi_call_wrapper(ConOut->SetCursorPosition, 3, ConOut, oldContext->column, oldContext->row);
			while (row < context.row) {
				while (col < columns) {
					uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L" ");
					col++;
				};
				col=0;
				row++;
			};
			col=0;
			while (col < context.column) {
				uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L" ");
				col++;
			}
		}
	}

	return uefi_call_wrapper(ConOut->SetCursorPosition, 3, ConOut, oldContext->column, oldContext->row);
}

/*
 *  \brief	reset the input console
 *
 *	reset the given input console
 *
 *	\param	ConIn	input console interface to reset
 *
 *	\return		the success state of the function
 */
EFI_STATUS reset_input(IN SIMPLE_INPUT_INTERFACE *ConIn) {
	ASSERT(ConIn != NULL);
	return uefi_call_wrapper(ConIn->Reset, 2, ConIn, FALSE);
}

/*
 *  \brief	check for ESC key
 *
 *	Check whether the user pressed the ESC key at the given input console. This
 *	function does NOT wait for a key. At the end, the input console is reset.
 *
 *	\param	ConIn	input console interface to reset
 *
 *	\return		TRUE: ESC was pressed, else FALSE
 */
BOOLEAN check_for_ESC(IN SIMPLE_INPUT_INTERFACE *ConIn) {
    EFI_STATUS error;
    EFI_INPUT_KEY key;

    ASSERT(ConIn != NULL);

    do {
    	error = uefi_call_wrapper(ConIn->ReadKeyStroke, 2, ConIn, &key);
		if (!EFI_ERROR(error)) {
		    if (key.ScanCode == 0x17) {
		    	reset_input(ConIn);
		    	return TRUE;
		    }
		}
    } while (error == EFI_SUCCESS);

	reset_input(ConIn);
	return FALSE;
}

/*
 *  \brief	dump the given percentage as progress of the process
 *
 *	The given value is is the per mill progress that is dumped to the screen.
 *	WORKAROUND: Since the decimal output (decimal to string) always crashes,
 *				a progress line is dumped instead the numerical output
 *
 *	\param	ConOut	output console interface to print to
 *	\param	value	the value in per mill
 *
 */
void dump_per_cent(IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, IN INTN value) {

	struct cs_output_context context = store_output_context(ConOut);
#if 0
	CHAR16 string[100];
#endif
	const UINTN len = 40;	/* length of the progress line */
	INTN i, v = (INTN)(value * len / 1000);

	if (pOptions->flags.silent)
		return;

	uefi_call_wrapper(ConOut->EnableCursor, 2, ConOut, FALSE); /* ignore return code this is not always supported */

	uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L"[");
	for (i = 0; i < len; i++) {
		if (i < v) {
			uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L"=");
		} else {
			uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L" ");
		}
	}
	uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L"]");

#if 0
	SPrint(string, 100, L"  %2.1d %%", value / 10); // this format does not work...
	uefi_call_wrapper(ConOut->OutputString, 2, ConOut, string);
#endif

	restore_output_context(ConOut, &context, FALSE);
}

/*
 *  \brief	return a language-dependent string based on the given ID
 *
 *	This function tries to transform the given ID into an unicode string.
 *	This is only possible for the IDs from "enum cs_enum_ui_stings" that are
 *	prepared for multiple languages.
 *
 *	\param	stringId	ID of the requested string
 *
 *	\return		unicode string (in case of error: empty string)
 */
CHAR16 *get_string(IN const enum cs_enum_ui_stings stringId) {
	UINTN i;

	ASSERT(pOptions != NULL);

	for (i = 0; ui_strings[i].id != 0; i++) {
		if (ui_strings[i].id == stringId) {
			return ui_strings[i].value[pOptions->language];
		}
	}
	return L"";
}

/*
 *  \brief	initialize user interface
 *
 *	This function is (still) unable to adjust the keyboard layout.
 *	It seems to be always English keyboard.
 *
 *	\param	ConIn		input console (simple input interface)
 *	\param	ConOut		output console (simple output text interface)
 *	\param	ConErr		error output console (simple output text interface)
 *
 *	\return		success state of the function
 */
static EFI_STATUS init_ui(IN SIMPLE_INPUT_INTERFACE *ConIn,
		IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConErr) {
    EFI_STATUS error;

	ASSERT(ConIn != NULL);
	ASSERT(ConOut != NULL);
	ASSERT(ConErr != NULL);
	currentMode = oldMode = ConOut->Mode->Mode;

	error = uefi_call_wrapper(ConOut->QueryMode, 4, ConOut, currentMode, &columns, &rows);
	if (EFI_ERROR(error)) return error;
	error = reset_input(ConIn);
	if (EFI_ERROR(error)) return error;

	return error;
}

/*
 * \brief	output a line of the given character
 *
 * The function takes the given character and prints a line of that characters
 * filling the whole line of screen.
 *
 *	\param	ConOut		output console (simple output interface)
 *	\param	character	the character that the line consists of (unicode)
 *
 *	\return		success state of the function
 * */
static EFI_STATUS print_line(IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, IN const CHAR16 character) {

	CHAR16 *line = AllocateZeroPool(columns * sizeof(CHAR16) + sizeof(CHAR16));

	if (pOptions->flags.silent)
		return EFI_SUCCESS;

	if (line) {
		EFI_STATUS error;
		int i;
		for (i = 0; i < columns; i++) {
			   line[i] = character;
		}
		error = uefi_call_wrapper(ConOut->OutputString, 2, ConOut, line);
		FreePool(line);
		if (EFI_ERROR(error)) {
		   return error;
		}
	}
	else {
		return EFI_OUT_OF_RESOURCES;
	}
	return EFI_SUCCESS;
}

/*
 *	\brief	wait for a single key pressed by the user
 *
 *	The function waits until the user pressed a key and returns the corresponding character/key.
 *
 *	\param	ConIn		input console (simple input interface)
 *	\param	key			buffer for last key that was pressed
 *
 *	\return		success state of the function
 */
static EFI_STATUS get_single_char(IN SIMPLE_INPUT_INTERFACE *ConIn, OUT EFI_INPUT_KEY *key) {
    WaitForSingleEvent(ConIn->WaitForKey, 0);
    return uefi_call_wrapper(ConIn->ReadKeyStroke, 2, ConIn, key);
}

/*
 *	\brief	read a string or key from input
 *
 *	The function reads from the input interface and fills the given buffer with the characters
 *	from the input. In case that the ENTER key, the ESCAPE or the [F8] key was pressed,
 *	the function returns.
 *	The caller need to detect the ESCAPE/F8 key by checking the returned value in the "key" buffer.
 *	If the buffer is exceeded, the function does not fail; instead, no more characters are appended
 *	to the string in the buffer.
 *	Depending on the flag "asciiMode" the buffer is interpreted as ASCII buffer or as UNICODE
 *	buffer returning ASCII strings (CHAR8 *) or UNICODE strings (CHAR16 *). In any case, the
 *	buffer_size gives the size in byte (not in characters). In case if ASCII mode, only valid
 *	ASCII characters are accepted, in UNICODE mode, no check of validity of the characters in
 *	the string is performed.
 *	The returned string is NULL-terminated, hence the given buffer_size shall take this into
 *	account.
 *	In case that ConOut is not NULL, then the string is given out in the following way:
 *	If dumpString is set, then the string is output, otherwise an asterisk character is
 *	output for each character.
 *
 *	\param	ConIn		input console (simple input interface)
 *	\param	ConOut		output console (simple text output interface), maybe NULL
 *	\param	buffer		output buffer for the string
 *	\param	buffer_size	size in byte of the buffer
 *	\param	stopAtF8	flag indicating whether the function exits if F8 key is pressed
 *	\param	dumpString	flag indicating that the typed text is displayed (only if ConOut is given)
 *	\param	asciiMode	flag indicating whether the buffer shall be filled with ASCII or UNICODE string
 *	\param	key			buffer for last key that was pressed
 *
 *	\return		success state of the function
 */
EFI_STATUS get_input(IN SIMPLE_INPUT_INTERFACE *ConIn, OPTIONAL SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut,
		OUT void *buffer, IN UINTN buffer_size, IN BOOLEAN stopAtF8,
		IN BOOLEAN dumpString, IN BOOLEAN asciiMode, OUT EFI_INPUT_KEY *key) {
    EFI_STATUS error;
    UINTN position = 0;
    UINTN positionMax;
    CHAR8 *asciiBuffer = (CHAR8 *)buffer;
    CHAR16 *unicodeBuffer = (CHAR16 *)buffer;

    ASSERT(ConIn != NULL);
    ASSERT(buffer != NULL);
    ASSERT(key != NULL);
    ASSERT(pOptions != NULL);

    positionMax = (asciiMode) ? buffer_size : buffer_size / sizeof(CHAR16);

    ASSERT(positionMax > 0);
    positionMax--;	/* reserve one character for the tailing \0 */
    SetMem(buffer, buffer_size, 0);

	for (;;) {
		/* wait for user input */
	    error = get_single_char(ConIn, key);
		if (EFI_ERROR (error)) continue;

		/* ESCAPE */
	    if (key->ScanCode == 0x17) {
	      break;
	    }
		/* ENTER */
	    if ((key->UnicodeChar == 0x0a) || (key->UnicodeChar == 0x0d)) {
	       break;
	    }
	    /* [F8] (only as first character of the string) */
	    if ((stopAtF8) && (key->ScanCode == 0x12) && (position == 0)) {
	      break;
	    }
	    /* backspace */
	    if (key->UnicodeChar == 0x08) {
	    	if (position > 0) {
	    		position--;
	    		if (asciiMode) {
	    			asciiBuffer[position] = '\0';
	    		} else {
	    			unicodeBuffer[position] = '\0';
	    		}
    			if ((!pOptions->flags.silent) && (ConOut != NULL)) {
    				INT32 column = ConOut->Mode->CursorColumn;

    				uefi_call_wrapper(ConOut->SetCursorPosition, 3,
    						ConOut, (column > 0) ? ConOut->Mode->CursorColumn - 1 : columns - 1,
    								(column > 0) ? ConOut->Mode->CursorRow : ConOut->Mode->CursorRow - 1);
    				uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L" ");
    				column = ConOut->Mode->CursorColumn;
    				uefi_call_wrapper(ConOut->SetCursorPosition, 3,
    						ConOut, (column > 0) ? ConOut->Mode->CursorColumn - 1 : columns - 1,
    								(column > 0) ? ConOut->Mode->CursorRow : ConOut->Mode->CursorRow - 1);
    			}
	    	}
	    } else {
	    	if ((asciiMode) && ((key->UnicodeChar <= 127) && (key->UnicodeChar >= 32))) {
	    		if (position < positionMax) {
		    		asciiBuffer[position] = (CHAR8)key->UnicodeChar;
		    		position++;

		    		if ((!pOptions->flags.silent) && (ConOut != NULL)) {
		    			if (dumpString) {
		    				uefi_call_wrapper(ConOut->OutputString, 2, ConOut, &key->UnicodeChar);
		    			} else {
		    				uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L"*");
		    			}
		    		}

	    		}
	    	} else if (!asciiMode) {
	    		if (position < positionMax) {
		    		unicodeBuffer[position] = key->UnicodeChar;
		    		position++;

		    		if ((!pOptions->flags.silent) && (ConOut != NULL)) {
		    			if (dumpString) {
		    				uefi_call_wrapper(ConOut->OutputString, 2, ConOut, &key->UnicodeChar);
		    			} else {
		    				uefi_call_wrapper(ConOut->OutputString, 2, ConOut, L"*");
		    			}
		    		}
	    		}
	    	}
	    }
	}

	return error;
}

/*
 *  \brief	ask the user "are you sure? (y/N)"
 *
 *	returns the user decision of the question "are you sure? (y/N)"
 *
 *	\param	ConIn	input console interface to read from
 *	\param	ConOut	output console interface to write to
 *
 *	\return		TRUE if the user typed 'y', otherwise FALSE (translations into other languages are supported)
 */
BOOLEAN check_really_do(IN SIMPLE_INPUT_INTERFACE *ConIn, IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut) {
    EFI_STATUS error;
    EFI_INPUT_KEY key;
    BOOLEAN doIt = FALSE;
    BOOLEAN done = FALSE;

    reset_input(ConIn);
	outStr(ConOut, L"\r\n ");
    outId(ConOut, CS_STR_ARE_YOU_SURE);

	for (;;) {
		/* wait for user input */
	    error = get_single_char(ConIn, &key);
		if (EFI_ERROR (error)) continue;

		/* ESCAPE (means (No) */
	    if (key.ScanCode == 0x17) {
	      break;
	    }
		/* ENTER (means (No) */
	    if ((key.UnicodeChar == 0x0a) || (key.UnicodeChar == 0x0d)) {
	       break;
	    }
	    switch (pOptions->language) {
	    case (CS_LANG_ENG):
	    	    if ((key.UnicodeChar == 'y') || (key.UnicodeChar == 'Y')) {
	    	    	done = doIt = TRUE;
	    	    } else if ((key.UnicodeChar == 'n') || (key.UnicodeChar == 'N')) {
	    	    	done = TRUE;
	    	    }
	    		break;
	    case (CS_LANG_GER):
				if ((key.UnicodeChar == 'j') || (key.UnicodeChar == 'J')) {
					done = doIt = TRUE;
	    	    } else if ((key.UnicodeChar == 'n') || (key.UnicodeChar == 'N')) {
	    	    	done = TRUE;
	    	    }
				break;
	    default:
	    	break;
	    }
	    if (done) {
	    	break;
	    }
	}
ret:
	if (EFI_ERROR(error)) {
    	CS_DEBUG((D_ERROR, L"check_really_do() error: 0x%x\n", error));
	}

	return doIt;
}

/*
 *	\brief	print message to ask for user password
 *
 *	The function prints a string for password request. Optionally a string is ouput
 *	before that shows that the previous password was wrong. This string is shown for
 *	a sort time and then overwritten by the password request phrase.
 *
 *	\param	ConOut		output console interface
 *	\param	showWrong	flag indicating whether the "wrong" string shall be shown before
 *	\param	pwdId		ID of the string to output
 *
 *	\return		the success state of the function
 */
EFI_STATUS ask_for_pwd(IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut, IN BOOLEAN showWrong,
		IN const enum cs_enum_ui_stings pwdId) {
    EFI_STATUS error;
    INT32 column, row;

    ASSERT(ConOut != NULL);

    column = ConOut->Mode->CursorColumn;
	row = ConOut->Mode->CursorRow;

	error = uefi_call_wrapper(ConOut->SetCursorPosition, 3, ConOut, 0, row);
	if (EFI_ERROR(error)) { return error; }
	error = print_line(ConOut, ' ');
	if (EFI_ERROR(error)) { return error; }
	error = uefi_call_wrapper(ConOut->SetCursorPosition, 3, ConOut, column, row);
	if (showWrong) {
		outId(ConOut, CS_STR_WRONG_PASSWD);
		cs_sleep(1);
		error = uefi_call_wrapper(ConOut->SetCursorPosition, 3, ConOut, 0, row);
		if (EFI_ERROR(error)) { return error; }
		error = print_line(ConOut, ' ');
		if (EFI_ERROR(error)) { return error; }
		error = uefi_call_wrapper(ConOut->SetCursorPosition, 3, ConOut, column, row);
		if (EFI_ERROR(error)) { return error; }
	}
	outId(ConOut, pwdId);

ret:
    return error;
}

/*
 *	\brief	dialog to process password change
 *
 *	The function provides the user dialog to change the password. After some checks the
 *	password change is performed. The dialog requests the old password and the new password
 *	for two times. The two new passwords are compared and the old password is checked.
 *	Only if these checks are passed the password is changed by modification of the
 *	volume header.
 *
 *	\param	ConIn		input console interface
 *	\param	ConOut		output console interface
 *	\param	user_decision	ID for the next activity in the process flow after returning from this function
 *	\param	passwd		buffer for the new password
 *
 *	\return		the success state of the function
 */
static EFI_STATUS change_pwd(IN SIMPLE_INPUT_INTERFACE *ConIn, IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut,
		OUT enum cs_enum_user_decision *user_decision, OUT Password *passwd) {
    EFI_STATUS error;
    EFI_INPUT_KEY key;
    Password newPwd, newPwd2;
    BOOLEAN newPwdDiffer;
    struct cs_output_context consoleContext;

	ASSERT(user_decision != NULL);
	ASSERT(pOptions != NULL);

	consoleContext = store_output_context(ConOut);	/* store current cursor position */

    do {
    	reset_input(ConIn);

    	restore_output_context(ConOut, &consoleContext, TRUE);	/* restore cursor position */

    	outStr(ConOut, L"\r\n\n  ");
    	/* first ask for the old password: */
    	error = ask_for_pwd(ConOut, FALSE, CS_STR_ENTER_OLD_PASSWD);
    	if (EFI_ERROR(error)) {
    		CS_DEBUG((D_ERROR, L"unable to output string (%r)\n", error));
    		goto ret;
    	}

    	error = get_input(ConIn, pOptions->flags.enable_password_asterisk ? ConOut : NULL,
    			&passwd->Text, sizeof(passwd->Text), FALSE /* F8 */, FALSE /* dump */, TRUE /* ASCII */, &key);
    	if (EFI_ERROR(error)) {
    		CS_DEBUG((D_ERROR, L"unable to read input string (%r)\n", error));
    		goto ret;
    	}

        if (key.ScanCode == 0x17) { /* ESCAPE */
        	*user_decision = CS_UI_SERVICE_MENU;
    		goto ret;
        }
        passwd->Length = strlena(passwd->Text);

    	/* now the new password: */
    	outStr(ConOut, L"\r\n  ");
    	error = ask_for_pwd(ConOut, FALSE, CS_STR_ENTER_NEW_PASSWD);
    	if (EFI_ERROR(error)) {
    		CS_DEBUG((D_ERROR, L"unable to output string (%r)\n", error));
    		goto ret;
    	}

    	error = get_input(ConIn, pOptions->flags.enable_password_asterisk ? ConOut : NULL,
    			&newPwd.Text, sizeof(newPwd.Text), FALSE /* F8 */, FALSE /* dump */, TRUE /* ASCII */, &key);
    	if (EFI_ERROR(error)) {
    		CS_DEBUG((D_ERROR, L"unable to read input string (%r)\n", error));
    		goto ret;
    	}

        if (key.ScanCode == 0x17) { /* ESCAPE */
        	*user_decision = CS_UI_SERVICE_MENU;
    		goto ret;
        }
        newPwd.Length = strlena(newPwd.Text);

        /* re-enter the new password: */
    	outStr(ConOut, L"\r\n  ");
    	error = ask_for_pwd(ConOut, FALSE, CS_STR_REENTER_PASSWD);
    	if (EFI_ERROR(error)) {
    		CS_DEBUG((D_ERROR, L"unable to output string (%r)\n", error));
    		goto ret;
    	}

    	error = get_input(ConIn, pOptions->flags.enable_password_asterisk ? ConOut : NULL,
    			&newPwd2.Text, sizeof(newPwd2.Text), FALSE /* F8 */, FALSE /* dump */, TRUE /* ASCII */, &key);
    	if (EFI_ERROR(error)) {
    		CS_DEBUG((D_ERROR, L"unable to read input string (%r)\n", error));
    		goto ret;
    	}

        if (key.ScanCode == 0x17) { /* ESCAPE */
        	*user_decision = CS_UI_SERVICE_MENU;
    		goto ret;
        }
        newPwd2.Length = strlena(newPwd2.Text);

        /* check/compare the new passwords: */
        if ((newPwd2.Length != newPwd.Length) || (strncmpa(newPwd2.Text, newPwd.Text, newPwd2.Length))) {
        	newPwdDiffer = TRUE;
        	outStr(ConOut, L"\r\n  ");
        	outId(ConOut, CS_STR_PASSWD_DIFFER);
    		cs_sleep(1);
        } else {
        	newPwdDiffer = FALSE;

        	/* now check the old password */
        	error = decrypt_volume_header();
        	if (error == EFI_ACCESS_DENIED) {
            	outStr(ConOut, L"\r\n  ");
            	outId(ConOut, CS_STR_WRONG_PASSWD);
        		cs_sleep(1);
            	error = EFI_ACCESS_DENIED;
        	}
        }

    } while ((newPwdDiffer) || (error == EFI_ACCESS_DENIED));

	if (!EFI_ERROR (error)) {

		/* set the new password... */
		error = change_password(&newPwd);

		if (!EFI_ERROR (error)) {
			*user_decision = CS_UI_MAIN_MENU;
		} else {
			CS_DEBUG((D_ERROR, L"unable to set new password: %r\n", error));
			outErr(ConOut, error);
    		cs_sleep(2);
		}
	} else {
		CS_DEBUG((D_ERROR, L"password change failed: %r\n", error));
		outErr(ConOut, error);
		cs_sleep(2);
	}

ret:
	SetMem(&newPwd, sizeof(&newPwd), 0);
	SetMem(&newPwd2, sizeof(&newPwd2), 0);

	return error;
}

/*
 *	\brief	user dialog for pre-boot authentication of the CipherShed EFI loader
 *
 *	The function provides the main user dialog of the CipherShed loader. Its main purpose
 *	is to get the user password. The resulting value "user_decision" defines the next step
 *	to be performed after this function.
 *
 *	\param	ConIn		input console interface
 *	\param	ConOut		output console interface
 *	\param	user_decision	ID for the next activity in the process flow after returning from this function
 *	\param	passwd		buffer for the user password
 *
 *	\return		the success state of the function
 */
static EFI_STATUS main_dialog(IN SIMPLE_INPUT_INTERFACE *ConIn, IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut,
		OUT enum cs_enum_user_decision *user_decision, OUT Password *passwd) {
    EFI_STATUS error;
    EFI_INPUT_KEY key;

	ASSERT(passwd != NULL);
	ASSERT(user_decision != NULL);
	ASSERT(pOptions != NULL);

	error = uefi_call_wrapper(ConOut->ClearScreen, 1, ConOut); /* might be deactivated in silent mode (?) */
	if (EFI_ERROR(error)) return error;

	uefi_call_wrapper(ConOut->EnableCursor, 2, ConOut, FALSE); /* ignore return code this is not always supported */

	outId(ConOut, CS_STR_MAIN_TITLE);
	outStr(ConOut, L"\r\n");

	error = print_line(ConOut, BOXDRAW_DOUBLE_HORIZONTAL);
	if (EFI_ERROR(error)) {
		return error;
	}
	outStr(ConOut, L"\r\n ");
	outId(ConOut, CS_STR_KBD_CONTROLS);
	outStr(ConOut, L"\r\n ");
	outId(ConOut, CS_STR_KBD_CONTROLS_UNDERLINE);
	outStr(ConOut, L"\r\n  ");
	outId(ConOut, CS_STR_ESC);
	if ((pOptions->flags.enable_service_menu) && (pOptions->flags.show_service_menu)) {
		outStr(ConOut, L"\r\n  ");
		outId(ConOut, CS_STR_SERVICE_MENU);
	}
	outStr(ConOut, L"\r\n\n ");
	outId(ConOut, CS_STR_ENTER_PASSWD);
	if (!pOptions->flags.silent)	/* maybe this condition is not needed... */
		uefi_call_wrapper(ConOut->EnableCursor, 2, ConOut, TRUE); /* ignore return code this is not always supported */

#if 0
	for (;;) {
		CHAR16 buf[100];

		error = get_input(ConIn, ConOut, buf, sizeof(buf), FALSE, FALSE /* UNICODE mode */, &key);
		if (EFI_ERROR(error)) return error;

	    if (key.ScanCode == 0x17) {
	    	Print(L"ESCAPE pressed\n");
	    } else if (key.ScanCode == 0x12) {
	       	Print(L"[F8] pressed\n");
		} else {
			Print(L"len: 0x%x\n",  StrLen(buf));
			Print(buf);
			Print(L"\n");
	    }
	}
#endif

	error = get_input(ConIn, pOptions->flags.enable_password_asterisk ? ConOut : NULL,
			&passwd->Text, sizeof(passwd->Text), pOptions->flags.enable_service_menu,
			FALSE /* dump the text to STDOUT */, TRUE /* ASCII mode */, &key);
	if (EFI_ERROR(error)) return error;

    if (key.ScanCode == 0x17) { /* ESCAPE */
    	*user_decision = CS_UI_EXIT_APP;
    } else if (key.ScanCode == 0x12) { /* [F8] -> call Service Menu */
    	*user_decision = CS_UI_SERVICE_MENU;
    } else {
    	passwd->Length = strlena(passwd->Text);
    	*user_decision = CS_UI_PASSWORD;
    }

ret:
	return error;
}

/* only for test purposes: allow to build arbitrary volume header */
#ifdef CS_TEST_CREATE_VOLUME_HEADER
extern EFI_STATUS create_new_volume_header();
#endif

/*
 *	\brief	main function of the service dialog
 *
 *	The function provides the service dialog of the CipherShed loader.
 *	The resulting value "user_decision" defines the next step
 *	to be performed after this function.
 *
 *	\param	ConIn		input console interface
 *	\param	ConOut		output console interface
 *	\param	user_decision	ID for the next activity in the process flow after returning from this function
 *
 *	\return		the success state of the function
 */
static EFI_STATUS service_dialog(IN SIMPLE_INPUT_INTERFACE *ConIn, IN SIMPLE_TEXT_OUTPUT_INTERFACE *ConOut,
		OUT enum cs_enum_user_decision *user_decision) {
    EFI_STATUS error;
    EFI_INPUT_KEY key;

    ASSERT(user_decision != NULL);

	error = uefi_call_wrapper(ConOut->ClearScreen, 1, ConOut);
	if (EFI_ERROR(error)) return error;

	uefi_call_wrapper(ConOut->EnableCursor, 2, ConOut, FALSE); /* ignore return code: this is not always supported */

	outId(ConOut, CS_STR_SERVICE_TITLE);
	outStr(ConOut, L"\r\n");

	error = print_line(ConOut, BOXDRAW_DOUBLE_HORIZONTAL);
	if (EFI_ERROR(error)) {
		return error;
	}
	outStr(ConOut, L"\r\n ");
	outId(ConOut, CS_STR_SERVICE_OPTIONS);
	outStr(ConOut, L"\r\n ");
	outId(ConOut, CS_STR_SERVICE_OPTIONS_UNDERLINE);
	if (pOptions->flags.enable_media_decryption) {
		outStr(ConOut, L"\r\n  ");
		outId(ConOut, CS_STR_SERVICE_DECRYPT_MEDIA);
	}
	if (pOptions->flags.enable_media_encryption) {
		outStr(ConOut, L"\r\n  ");
		outId(ConOut, CS_STR_SERVICE_ENCRYPT_MEDIA);
	}
	if (pOptions->flags.enable_password_change) {
		outStr(ConOut, L"\r\n  ");
		outId(ConOut, CS_STR_SERVICE_CHANGE_PASSWORD);
	}
#ifdef CS_TEST_CREATE_VOLUME_HEADER
	outStr(ConOut, L"\r\n  ");
	outStr(ConOut, L"[4]    *** build own volume header ***");
#endif
	outStr(ConOut, L"\r\n  ");
	outId(ConOut, CS_STR_SERVICE_ESC);
	outStr(ConOut, L"\r\n\n");
	outId(ConOut, CS_STR_SERVICE_SELECT);
	if (!pOptions->flags.silent)
		uefi_call_wrapper(ConOut->EnableCursor, 2, ConOut, TRUE); /* ignore return code: this is not always supported */

	while (*user_decision == CS_UI_SERVICE_MENU) {
		/* wait for user input */
	    error = get_single_char(ConIn, &key);
		if (EFI_ERROR (error)) continue;

		/* ESCAPE */
	    if (key.ScanCode == 0x17) {
	    	*user_decision = CS_UI_MAIN_MENU;
	      break;
	    }
	    switch (key.UnicodeChar) {
	    case '1':
	    	outStr(ConOut, &key.UnicodeChar);
	    	if (pOptions->flags.enable_media_decryption) {
		    	*user_decision = CS_UI_DECRYPT_SYSTEM;
	    	}
	    	break;
	    case '2':
	    	outStr(ConOut, &key.UnicodeChar);
	    	if (pOptions->flags.enable_media_encryption) {
		    	*user_decision = CS_UI_ENCRYPT_SYSTEM;
	    	}
	    	break;
	    case '3':
	    	outStr(ConOut, &key.UnicodeChar);
	    	if (pOptions->flags.enable_password_change) {
		    	*user_decision = CS_UI_CHANGE_PASSWD;
	    	}
	    	break;
#ifdef CS_TEST_CREATE_VOLUME_HEADER
	    case '4':
	    	outStr(ConOut, &key.UnicodeChar);
	    	outStr(ConOut, L"\r\n");
	    	error = create_new_volume_header();
	    	*user_decision = CS_UI_EXIT_APP;
	    	break;
#endif
	    default:
	    	break;
	    }
	}

ret:
    return error;
}

/*
 *	\brief	entry point of the user dialog for pre-boot authentication of the CipherShed EFI loader
 *
 *	The function provides the entry point for the user interface of the CipherShed loader. Its main purpose
 *	is the flow control of the user interface and the call of the corresponding dialog functions.
 *
 *	\param	ImageHandle		image handle
 *	\param	SystemTable		system table
 *	\param	options			pointer to the user options defined in the configuration file
 *	\param	user_decision	ID for the next activity in the process flow after returning from this function
 *	\param	passwd		buffer for the user password
 *
 *	\return		the success state of the function
 */
EFI_STATUS user_dialog(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,
		IN struct cs_option_data *options,
		OUT enum cs_enum_user_decision *user_decision, OUT Password *passwd) {
    EFI_STATUS error;
	SIMPLE_INPUT_INTERFACE  *ConIn;
	SIMPLE_TEXT_OUTPUT_INTERFACE  *ConOut, *ConErr;

	ASSERT(user_decision != NULL);
	ASSERT(SystemTable != NULL);
    ASSERT(options != NULL);

    pOptions = options;	/* initialize global variable */

	ConIn = SystemTable->ConIn;
	ConOut = SystemTable->ConOut;
	ConErr = SystemTable->StdErr;
	error = init_ui(ConIn, ConOut, ConErr);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"unable to initialize user interface (%r)\n", error));
		return error;
	}

	error = main_dialog(ConIn, ConOut, user_decision, passwd);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"main user dialog failed (%r)\n", error));
		return error;
	}

	while (*user_decision == CS_UI_SERVICE_MENU) {
		error = service_dialog(ConIn, ConOut, user_decision);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"service dialog failed (%r)\n", error));
			return error;
		}
		switch (*user_decision) {

		case CS_UI_DECRYPT_SYSTEM:
			error = encrypt_decrypt_media(ImageHandle, SystemTable, options, FALSE /* decrypt */,
					user_decision, passwd);
		break;

		case CS_UI_ENCRYPT_SYSTEM:
			error = encrypt_decrypt_media(ImageHandle, SystemTable, options, TRUE /* encrypt */,
					user_decision, passwd);
		break;

		case CS_UI_CHANGE_PASSWD:
			error = change_pwd(ConIn, ConOut, user_decision, passwd);
		break;

		default:
			break;
		}
	}

	return error;
}
