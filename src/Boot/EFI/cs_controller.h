/*  cs_controller.h - CipherShed EFI boot loader
 *
 *
 *
 */

#ifndef _CS_CONTROLLER_H_
#define _CS_CONTROLLER_H_

#include "cs_common.h"

EFI_STATUS change_password(IN Password *password);
EFI_STATUS update_volume_header(IN CRYPTO_INFO *newCryptoInfo);
void set_volume_header_cipher(IN PCRYPTO_INFO cryptoInfo);
EFI_HANDLE get_boot_partition_handle();
CRYPTO_INFO *get_crypto_info();
EFI_STATUS decrypt_volume_header();
EFI_STATUS start_connect_fake_crypto_driver(IN EFI_HANDLE ImageHandle);
UINTN read_file(IN EFI_FILE_HANDLE root_handle, IN CHAR16 *filename, OUT CHAR8 **content);

#endif /* _CS_CONTROLLER_H_ */
