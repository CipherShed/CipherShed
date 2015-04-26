/*  cs_service.h - CipherShed EFI boot loader
 *
 *
 *
 */

#ifndef _CS_SERVICE_H_
#define _CS_SERVICE_H_

#include "cs_common.h"
#include "cs_ui.h"

EFI_STATUS encrypt_decrypt_media(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,
		IN const struct cs_option_data *options, IN BOOLEAN encrypt,
		OUT enum cs_enum_user_decision *user_decision, OUT Password *passwd);


#endif /* _CS_SERVICE_H_ */
