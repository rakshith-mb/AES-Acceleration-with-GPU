/******************************************************************************
 * File Name    - aes_naive.h
 * 
 * Description  - This is the header file for the aes_naive code
 ******************************************************************************/

#ifndef SOURCE_AES_NAIVE_H_
#define SOURCE_AES_NAIVE_H_

#include "main.h"

/*******************************************************************************
* Global constants
*******************************************************************************/
#define AES128_KEY_SIZE             16
#define AES256_KEY_SIZE             32

#define AES128_ROUNDS               11
#define AES256_ROUNDS               15

// 16 byte keys are created
#define AES128_ROUND_KEY_LENGTH     16*AES128_ROUNDS
#define AES256_ROUND_KEY_LENGTH     16*AES256_ROUNDS

/*******************************************************************************
* Structures and enumerations
*******************************************************************************/
typedef struct aes_struct
{
    uint8_t aes_mode;                                   // AES_ECB or AES_CTR
    uint8_t aes_key_length;                             // In bits - 128 or 256
    const uint8_t* key;                                 // Buffer to store AES key
    uint8_t* round_key;                                 // Buffer to store round key
    uint8_t* counter;                                   // Buffer to store IV in CTR mode
    uint8_t* plain_text;                                // Buffer to store plain text
    int plain_text_length;                              // In bytes
    uint8_t* cipher_text;                               // Buffer to store cipher text
} aes_struct;


/*******************************************************************************
* Function prototypes
*******************************************************************************/
void aes_init(aes_struct* aes_config_struct);
uint8_t aes_sbox_get_val(uint8_t byte_val);
void aes_encrypt_buffer(aes_struct* aes_config_struct);
void aes_encrypt_ecb(aes_struct* aes_config_struct);
void aes_encrypt_ctr(aes_struct* aes_config_struct);
void aes_encrypt_state(uint8_t* state_ptr_cipher_text, const uint8_t* state_ptr_plain_text, uint8_t key_length, uint8_t* round_key);
void aes_add_round_key(uint8_t* buffer, uint8_t* round_key);
void aes_sub_bytes(uint8_t* buffer);
void aes_shift_rows(uint8_t* buffer);
void aes_mix_columns(uint8_t* buffer);
uint8_t aes_galoi_mult(uint8_t num, uint8_t mult);

#endif /* SOURCE_AES_NAIVE_H_ */

/* [] END OF FILE */