#ifndef _SHE_EXTENDED_H_
#define _SHE_EXTENDED_H_

/*===========================================================================

   Edit History:
   
When       Who     What, where, why
--------   ---     ----------------------------------------------------------
02/21/23    SG      Created.

=============================================================================

  Copyright (c) 2023 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  
===========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif

/*===========================================================================
    Includes
============================================================================*/
#include <stdint.h>
#include <stdbool.h>
/*===========================================================================
    Definitions
 ============================================================================*/
/** @addtogroup she_datatypes
@{ */

/**
  Return values of SHE APIs.
 */
typedef enum {
  ERC_NO_ERROR              = 0,
  /**< Successful error code. */

  ERC_GENERAL_ERROR         = -1,
  /**< General error code for failures that are not specified below */

  ERC_SEQUENCE_ERROR        = -2,
  /**< Any SHE API shall return this error code if it is called before she_init(). */

  ERC_KEY_EMPTY             = -3,
  /**< The requested key is not provisioned. */

  ERC_KEY_INVALID           = -4,
  /**< The security flags of the requested key do not allow the operation. */

  ERC_MEMORY_FAILURE        = -5,
  /**< The API failed to read/write to key table in non-volatile memory. */


  ERC_KEY_UPDATE_ERROR      = -6,
  /**< Key provisioning failure due to messsage verification failure, bad UID,
       or wrong counter. */

  ERC_KEY_WRITE_PROTECTED   = -7,
  /**< Key provisioning failure due to write protection enabled in security flag. */
} she_erc_t;
/** @} *//* end_addtogroup she_datatypes */

/**
  Context gvm 
*/
typedef enum {
    SHE_GVM_NONE = 0,
    SHE_LA_GVM = 1,
} she_ctx_gvm;

struct _she_context;
typedef struct _she_context she_context_t;


/*===========================================================================
    Functions
 ============================================================================*/

/** @addtogroup she_functions
@{ */

/**
  @xreflabel{hdr:SHEContextCreate}
  Creates a SHE execution context.
  Use different contexts for calling SHE APIs concurrently from different threads.

  @param [out] context  Pointer to be passed when calling SHE APIs in the same
                        context.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_GENERAL_ERROR on failure. @newpage
 */
she_erc_t she_context_create(she_context_t** context);


/**
  @xreflabel{hdr:SHEContextCreate}
  Creates a SHE execution context for gvm. Should only be used from PVM.
  Use different contexts for calling SHE APIs concurrently from different threads.

  @param [out] context  Pointer to be passed when calling SHE APIs in the same
                        context.
  @param [in] type  Type of context to be created

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_GENERAL_ERROR on failure or when called from GVM. @newpage
 */
she_erc_t she_context_gvm_create(she_context_t** context, she_ctx_gvm gvm);

/**
  @xreflabel{hdr:SHEContextDestroy}
  Destroys a SHE execution context.

  @param [in] context  Pointer to the context to be destroyed.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_GENERAL_ERROR on failure. @newpage
 */
she_erc_t she_context_destroy(she_context_t* context);

/**
  @xreflabel{hdr:SHEInit}
  Initializes a SHE context.

  @param [in] context  Pointer to the context of execution.

  @detdesc
  This function must be called after she_context_create() and before any other
  SHE API in this context.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_MEMORY_FAILURE on read failure from the key table in the non-volatile
    memory.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_init(she_context_t* context);

/**
  Encrypts AES-128 ECB mode using a key in key bank 1.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank1(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 1.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank1(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 1.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank1(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 1.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank1(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 1.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank1(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 1.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank1( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeyBank1}
  Provisions key to key bank 1.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 1 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank1(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 2.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_ecb_bank2(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 2.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank2(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 2.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext.
  @param [in] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank2(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 2.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in the plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank2(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 2.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank2(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 2.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank2( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeyBank2}
  Provisions key to key bank 2

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 1 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank2(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 3.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_ecb_bank3(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 3.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank3(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 3.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext.
  @param [in] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank3(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 3.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in the plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank3(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 3.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank3(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 3.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank3( she_context_t*    context,
                                const uint8_t     key_id,
                                const uint8_t*    message,
                                const uint64_t    message_length,
                                const uint8_t*    mac,
                                const uint8_t     mac_length,
                                bool*             verification_status);

/**
  Provisions key to key bank 3.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 1 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank3(   she_context_t*   context,
                                const uint8_t*   m1,
                                const uint8_t*   m2,
                                const uint8_t*   m3,
                                uint8_t*         m4,
                                uint8_t*         m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 4.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_ecb_bank4(she_context_t*    context,
                            const uint8_t     key_id,
                            const uint8_t*    plaintext,
                            uint8_t*          cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 4.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank4(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 4.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext.
  @param [in] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank4(she_context_t*    context,
                            const uint8_t     key_id,
                            uint8_t*          plaintext,
                            const uint8_t*    cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 4.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in the plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128-bits plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank4(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 4.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank4(   she_context_t*    context,
                                    const uint8_t     key_id,
                                    const uint8_t*    message,
                                    const uint64_t    message_length,
                                    uint8_t*          mac);

/**
  Verifies MAC verification using a key in key bank 4.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank4( she_context_t*    context,
                                const uint8_t     key_id,
                                const uint8_t*    message,
                                const uint64_t    message_length,
                                const uint8_t*    mac,
                                const uint8_t     mac_length,
                                bool*             verification_status);

/**
  Provisions key to key bank 4.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 1 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank4(   she_context_t*   context,
                                const uint8_t*   m1,
                                const uint8_t*   m2,
                                const uint8_t*   m3,
                                uint8_t*         m4,
                                uint8_t*         m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 5.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext
  @param [out] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_ecb_bank5(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 5.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank5(she_context_t*    context,
                            const uint8_t     key_id,
                            const uint8_t*    iv,
                            const uint32_t    n,
                            const uint8_t*    plaintext,
                            uint8_t*          cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 5.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext.
  @param [in] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank5(she_context_t*    context,
                            const uint8_t     key_id,
                            uint8_t*          plaintext,
                            const uint8_t*    cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 5.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in the plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank5(she_context_t*    context,
                            const uint8_t     key_id,
                            const uint8_t*    iv,
                            const uint32_t    n,
                            uint8_t*          plaintext,
                            const uint8_t*    cyphertext);

/**
  Generates MAC using a key in key bank 5.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank5(   she_context_t*    context,
                                    const uint8_t     key_id,
                                    const uint8_t*    message,
                                    const uint64_t    message_length,
                                    uint8_t*          mac);

/**
  Verifies MAC using a key in key bank 5.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
  - operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank5( she_context_t*    context,
                                const uint8_t     key_id,
                                const uint8_t*    message,
                                const uint64_t    message_length,
                                const uint8_t*    mac,
                                const uint8_t     mac_length,
                                bool*             verification_status);

/**
  Provisions key to key bank 5

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 1 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank5(   she_context_t*    context,
                                const uint8_t*    m1,
                                const uint8_t*    m2,
                                const uint8_t*    m3,
                                uint8_t*          m4,
                                uint8_t*          m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 6.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank6(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 6.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank6(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 6.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank6(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 6.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank6(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 6.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank6(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 6.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank6( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeyBank6}
  Provisions key to key bank 6.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 6 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank6(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 7.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank7(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 7.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank7(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 7.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank7(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 7.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank7(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 7.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank7(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 7.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank7( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeyBank7}
  Provisions key to key bank 7.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 7 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank7(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 8.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank8(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 8.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank8(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 8.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank8(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 8.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank8(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 8.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank8(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 8.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank8( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeyBank8}
  Provisions key to key bank 8.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 8 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank8(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 9.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank9(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 9.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank9(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 9.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank9(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 9.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank9(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 9.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank9(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 9.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank9( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank9}
  Provisions key to key bank 9.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 9 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank9(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 10.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank10(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 10.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank10(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 10.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank10(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 10.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank10(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 10.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank10(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 10.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank10( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank10}
  Provisions key to key bank 10.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 10 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank10(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);                                

/**
  Encrypts AES-128 ECB mode using a key in key bank 11.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank11(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 11.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank11(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 11.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank11(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 11.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank11(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 11.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank11(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 11.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank11( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank11}
  Provisions key to key bank 11.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 11 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank11(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 12.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank12(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 12.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank12(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 12.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank12(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 12.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank12(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 12.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank12(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 12.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank12( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank12}
  Provisions key to key bank 12.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 12 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank12(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 13.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank13(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 13.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank13(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 13.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank13(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 13.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank13(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 13.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank13(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 13.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank13( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank13}
  Provisions key to key bank 13.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 13 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank13(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 14.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank14(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 14.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank14(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 14.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank14(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 14.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank14(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 14.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank14(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 14.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank14( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank14}
  Provisions key to key bank 14.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 14 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank14(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 15.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank15(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 15.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank15(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 15.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank15(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 15.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank15(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 15.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank15(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 15.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank15( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank15}
  Provisions key to key bank 15.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 15 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank15(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 16.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank16(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 16.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank16(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 16.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank16(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 16.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank16(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 16.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank16(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 16.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank16( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank16}
  Provisions key to key bank 16.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 16 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank16(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 17.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank17(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 17.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank17(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 17.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank17(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 17.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank17(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 17.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank17(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 17.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank17( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank17}
  Provisions key to key bank 17.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 17 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank17(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 18.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank18(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 18.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank18(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 18.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank18(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 18.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank18(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 18.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank18(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 18.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank18( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank18}
  Provisions key to key bank 18.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 18 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank18(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 19.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank19(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 19.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank19(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 19.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank19(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 19.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank19(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 19.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank19(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 19.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank19( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank19}
  Provisions key to key bank 19.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 19 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank19(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  Encrypts AES-128 ECB mode using a key in key bank 20.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] plaintext    128 bit plaintext.
  @param [out] cyphertext  128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failures. @newpage
 */
she_erc_t she_enc_ecb_bank20(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Encrypts AES-128 CBC mode using a key in key bank 20.

  @param [in] context      Pointer to the context of execution.
  @param [in] key_id       4 bit key ID.
  @param [in] iv           128 bit initialization vector.
  @param [in] n            Number of 128 bit blocks in plaintext
                           (0 < n @le 4096).
  @param [in] plaintext    n*128 bit plaintext.
  @param [out] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_enc_cbc_bank20(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            const uint8_t*     plaintext,
                            uint8_t*           cyphertext);

/**
  Decrypts AES-128 ECB mode using a key in key bank 20.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [out] plaintext  128 bit plaintext
  @param [in] cyphertext  128 bit cyphertext

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_ecb_bank20(she_context_t*     context,
                            const uint8_t      key_id,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Decrypts AES-128 CBC mode using a key in key bank 20.

  @param [in] context     Pointer to the context of execution.
  @param [in] key_id      4 bit key ID.
  @param [in] iv          128 bit initialization vector.
  @param [in] n           Number of 128 bit blocks in plaintext
                          (0 < n @le 4096).
  @param [out] plaintext  n*128 bit plaintext.
  @param [in] cyphertext  n*128 bit cyphertext.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_dec_cbc_bank20(she_context_t*     context,
                            const uint8_t      key_id,
                            const uint8_t*     iv,
                            const uint32_t     n,
                            uint8_t*           plaintext,
                            const uint8_t*     cyphertext);

/**
  Generates MAC using a key in key bank 20.

  @param [in] context         Pointer to the context of execution.
  @param [in] key_id          4 bit key ID.
  @param [in] message         n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length  Length of the message in bits (should be byte aligned).
  @param [out] mac            128 bit MAC of the message.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_generate_mac_bank20(   she_context_t*     context,
                                    const uint8_t      key_id,
                                    const uint8_t*     message,
                                    const uint64_t     message_length,
                                    uint8_t*           mac);

/**
  Verifies MAC using a key in key bank 20.

  @param [in] context               Pointer to the context of execution.
  @param [in] key_id                4 bit key ID.
  @param [in] message               n*128 bit message, where n=CEIL(MESSAGE_LENGTH / 128).
  @param [in] message_length        Length of the message in bits (should be
                                    byte aligned).
  @param [in] mac                   128 bit MAC of the message.
  @param [in] mac_length            Number of bits to compare, starting from the
                                    leftmost bit of MAC.
  @param [out] verification_status  Verification status. FALSE, if
                                    message_length=0 with no error returned.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key.
  - #ERC_KEY_INVALID if security flags of the requested key do not allow the
    operation.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_verify_mac_bank20( she_context_t*     context,
                                const uint8_t      key_id,
                                const uint8_t*     message,
                                const uint64_t     message_length,
                                const uint8_t*     mac,
                                const uint8_t      mac_length,
                                bool*              verification_status);

/**
  @xreflabel{hdr:LoadKeybank20}
  Provisions key to key bank 20.

  @param [in] context  Pointer to the context of execution.
  @param [in] m1       128 bit M1 as defined in SHE specification.
  @param [in] m2       256 bit M2 as defined in SHE specification.
  @param [in] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4      256 bit M4 as defined in SHE specification.
  @param [out] m5      128 bit M5 as defined in SHE specification.

  @detdesc
  m1, m2 and m3 are the input data for provisioning. m4 and m5 are returned
  and used to verify that provisioning was successful.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_KEY_EMPTY when called with an unprovisioned key or key ID not between
    0x4 and 0xd. Key IDs 0x1 and 0xe in key bank 20 are exceptions and will not
    return an empty error.
  - #ERC_KEY_UPDATE_ERROR for failure because of message verification, bad UID,
    or a wrong counter.
  - #ERC_KEY_WRITE_PROTECTED for failure because of a write protection was
    enabled in a security flag.
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_key_bank20(   she_context_t*     context,
                                const uint8_t*     m1,
                                const uint8_t*     m2,
                                const uint8_t*     m3,
                                uint8_t*           m4,
                                uint8_t*           m5);

/**
  @xreflabel{hdr:SHECommit}
  Writes the provisioned keys into non-volatile memory.

  @param [in] context  Pointer to the context of execution.

  @detdesc
  If she_commit() is not used after provisioning, the keys are only updated in
  RAM and the next boot will load the old keys from RPMB.
  @par
  This function saves the keys to RPMB. After she_init() is called, these keys
  are loaded to RAM. Keys in non-volatile memory can overcome a bit flip due to
  redundancy.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_MEMORY_FAILURE on read failure from the key table in the non-volatile
    memory.
  - #ERC_GENERAL_ERROR on any other failure. @newpage

 */
she_erc_t she_commit(she_context_t* context);

/**
  @xreflabel{hdr:SHELoadPlain}
  Loads plain RAM key and sets key to volatile memory.

  @param [in] context    Pointer to the context of execution.
  @param [in] plain_key  Buffer containing 128 bit key in plain text.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_load_plain_key(she_context_t* context, const uint8_t* plain_key);

/**
  @xreflabel{hdr:SHEexportRAM}
  Exports RAM key and provides a set of provisioning messages for provisioning
  RAM key in the future.

  @param [in] context   Pointer to the context of execution.
  @param [out] m1       128 bit M1 as defined in SHE specification.
  @param [out] m2       256 bit M2 as defined in SHE specification.
  @param [out] m3       128 bit M3 as defined in SHE specification.
  @param [out] m4       256 bit M4 as defined in SHE specification.
  @param [out] m5       128 bit M5 as defined in SHE specification.

  @detdesc
  These messages must be used only with she_load_key_bank1(), and not with any
  other key bank. Only the RAM key that was loaded as plain text using
  she_load_plain_key() can be exported.

  @returns
  - #ERC_NO_ERROR on success.
  - #ERC_SEQUENCE_ERROR if called before she_init().
  - #ERC_GENERAL_ERROR on any other failure. @newpage
 */
she_erc_t she_export_ram_key(she_context_t*   context,
                             uint8_t*         m1,
                             uint8_t*         m2,
                             uint8_t*         m3,
                             uint8_t*         m4,
                             uint8_t*         m5);

#ifdef __cplusplus
}
/** @} *//* end_addtogroup she_functions */

#endif
#endif //_SHE_EXTENDED_H_

