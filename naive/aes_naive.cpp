/******************************************************************************
 * File Name    - aes_naive.cpp
 * 
 * Description  - This cpp file contains the function definitions of all aes 
 *                helper functions
 ******************************************************************************/
#include "string.h"
#include "aes_naive.h"

/*******************************************************************************
* Global constants
*******************************************************************************/

static const uint8_t sbox[256] = {
    /*          0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
    /* 0 */  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    /* 1 */  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    /* 2 */  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    /* 3 */  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    /* 4 */  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    /* 5 */  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    /* 6 */  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    /* 7 */  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    /* 8 */  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    /* 9 */  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    /* A */  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    /* B */  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    /* C */  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    /* D */  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    /* E */  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    /* F */  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 
    };

// Helper function to get the S-Box value
uint8_t aes_sbox_get_val(uint8_t byte_val)
{
    uint8_t row = (byte_val & 0xF0) >> 4;
    uint8_t col = (byte_val & 0x0F);

    return sbox[row*16 + col];
}

// Function to initialize the AES config structure
void aes_init(aes_struct* aes_config_struct)
{
    aes_config_struct->aes_mode = AES_MODE;
    aes_config_struct->aes_key_length = AES_KEY_SIZE;

    if(aes_config_struct->aes_key_length == AES128_KEY_SIZE*8)
    {
        aes_config_struct->round_key = new uint8_t[AES128_ROUND_KEY_LENGTH];
    }
    else
    {
        aes_config_struct->round_key = new uint8_t[AES256_ROUND_KEY_LENGTH];
    }
}

// Function to launch the appropriate AES mode
void aes_encrypt_buffer(aes_struct* aes_config_struct)
{
    if(aes_config_struct->aes_mode == AES_ECB)
    {
        aes_encrypt_ecb(aes_config_struct);
    }
    else
    {
        // aes_encrypt_ctr(aes_config_struct);
    }
}

// Function to encrypt the buffer in ECB mode
void aes_encrypt_ecb(aes_struct* aes_config_struct)
{
    uint8_t* state_ptr_plain_text = aes_config_struct->plain_text; 
    uint8_t* state_ptr_cipher_text = aes_config_struct->cipher_text;

    for(int i = 0; i < aes_config_struct->plain_text_length; i = i + AES_BLK_LENGTH)
    {
        aes_encrypt_state(state_ptr_cipher_text, state_ptr_plain_text, aes_config_struct->aes_key_length, aes_config_struct->round_key);
        state_ptr_cipher_text += AES_BLK_LENGTH;
        state_ptr_plain_text += AES_BLK_LENGTH;
    }
}

// Function to compute AES encryption per block
void aes_encrypt_state(uint8_t* state_ptr_cipher_text, const uint8_t* state_ptr_plain_text, uint8_t key_length, uint8_t* round_key)
{
    int curr_round = 0, num_rounds;
    uint8_t* round_key_ptr = round_key;

    memcpy(state_ptr_cipher_text, state_ptr_plain_text, AES_BLK_LENGTH);

    // printf("Printing values...\n");
    // for(int i = 0; i < AES_BLK_LENGTH; i++)
    // {
    //     printf("0x%02x ", state_ptr_cipher_text[i]);
    // }

    if(key_length == AES128_KEY_SIZE*8)
    {
        num_rounds = AES128_ROUNDS;
    }
    else
    {
        num_rounds = AES256_ROUNDS;
    }

    // Add round key step prior to the first round
    aes_add_round_key(state_ptr_cipher_text, round_key_ptr);
    round_key_ptr+=AES_BLK_LENGTH;
    curr_round++;

    for(; curr_round < num_rounds - 1; curr_round++)
    {
        // Substitute bytes step
        aes_sub_bytes(state_ptr_cipher_text);

        // Shift rows step
        aes_shift_rows(state_ptr_cipher_text);

        // Mix Columns step
        aes_mix_columns(state_ptr_cipher_text);

        // Add Round Key step
        aes_add_round_key(state_ptr_cipher_text, round_key_ptr);
        round_key_ptr+=AES_BLK_LENGTH;
    }

    // Last round: Without matrix multiplication
    aes_sub_bytes(state_ptr_cipher_text);
    aes_shift_rows(state_ptr_cipher_text);
    aes_add_round_key(state_ptr_cipher_text, round_key_ptr);
}

// Funtion for Add Round Key step
void aes_add_round_key(uint8_t* buffer, uint8_t* round_key)
{
    // printf("Printing values...\n");
    // for(int i = 0; i < AES_BLK_LENGTH; i++)
    // {
    //     printf("0x%02x ", buffer[i]);
    // }
    for(int i = 0; i < 16; i++)
    {
        buffer[i] ^= round_key[i];
    }
}

// Funtion for Substitute Bytes step
void aes_sub_bytes(uint8_t* buffer)
{
    for(int i = 0; i < 16; i++)
    {
        buffer[i] = aes_sbox_get_val(buffer[i]);
    }
}

// Funtion for Shift Rows step
void aes_shift_rows(uint8_t* buffer)
{
    uint8_t temp_var;

    // printf("\n\nPrinting values...\n");
    // for(int i = 0; i < AES_BLK_LENGTH; i++)
    // {
    //     printf("0x%02x ", buffer[i]);
    // }

    temp_var = buffer[1];
    buffer[1] = buffer[5];
    buffer[5] = buffer[9];
    buffer[9] = buffer[13];
    buffer[13] = temp_var;

    temp_var = buffer[2];
    buffer[2] = buffer[10];
    buffer[10] = temp_var;
    temp_var = buffer[6];
    buffer[6] = buffer[14];
    buffer[14] = temp_var;

    temp_var = buffer[3];
    buffer[3] = buffer[15];
    buffer[15] = buffer[11];
    buffer[11] = buffer[7];
    buffer[7] = temp_var;

    // printf("\n\nPrinting shifted values...\n");
    // for(int i = 0; i < AES_BLK_LENGTH; i++)
    // {
    //     printf("0x%02x ", buffer[i]);
    // }
}

// Funtion for Mix Columns step
// Reference - https://crypto.stackexchange.com/questions/2402/how-to-solve-mixcolumns
void aes_mix_columns(uint8_t* buffer)
{
    uint8_t* temp_buf = new uint8_t[AES_BLK_LENGTH];

    memcpy(temp_buf, buffer, AES_BLK_LENGTH);

    // printf("\n\nPrinting values...\n");
    // for(int i = 0; i < AES_BLK_LENGTH; i++)
    // {
    //     printf("0x%02x ", temp_buf[i]);
    // }
    // printf("\n\n");

    for(int i = 0; i < 4; i++)
    {
        // printf("\n0x%02x 0x%02x 0x%02x 0x%02x \n", temp_buf[i * 4 + 0], temp_buf[i * 4 + 1], temp_buf[i * 4 + 2], temp_buf[i * 4 + 3]);
        buffer[i * 4 + 0] = (aes_galoi_mult(temp_buf[i * 4 + 0], 2) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 1], 3) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 2], 1) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 3], 1));
        // printf("\n0x%02x 0x%02x 0x%02x 0x%02x \n", aes_galoi_mult(temp_buf[i * 4 + 0], 2), aes_galoi_mult(temp_buf[i * 4 + 1], 3), aes_galoi_mult(temp_buf[i * 4 + 2], 1), aes_galoi_mult(temp_buf[i * 4 + 3], 1));

        buffer[i * 4 + 1] = (aes_galoi_mult(temp_buf[i * 4 + 0], 1) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 1], 2) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 2], 3) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 3], 1));
        // printf("\n0x%02x 0x%02x 0x%02x 0x%02x \n", aes_galoi_mult(temp_buf[i * 4 + 0], 1), aes_galoi_mult(temp_buf[i * 4 + 1], 2), aes_galoi_mult(temp_buf[i * 4 + 2], 3), aes_galoi_mult(temp_buf[i * 4 + 3], 1));

        buffer[i * 4 + 2] = (aes_galoi_mult(temp_buf[i * 4 + 0], 1) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 1], 1) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 2], 2) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 3], 3));
        // printf("\n0x%02x 0x%02x 0x%02x 0x%02x \n", aes_galoi_mult(temp_buf[i * 4 + 0], 1), aes_galoi_mult(temp_buf[i * 4 + 1], 1), aes_galoi_mult(temp_buf[i * 4 + 2], 2), aes_galoi_mult(temp_buf[i * 4 + 3], 3));

        buffer[i * 4 + 3] = (aes_galoi_mult(temp_buf[i * 4 + 0], 3) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 1], 1) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 2], 1) ^ 
                             aes_galoi_mult(temp_buf[i * 4 + 3], 2));
        // printf("\n0x%02x 0x%02x 0x%02x 0x%02x \n", aes_galoi_mult(temp_buf[i * 4 + 0], 3), aes_galoi_mult(temp_buf[i * 4 + 1], 1), aes_galoi_mult(temp_buf[i * 4 + 2], 1), aes_galoi_mult(temp_buf[i * 4 + 3], 2));

        // printf("\n****************************\n");
    }

    // printf("\n\nPrinting shifted values...\n");
    // for(int i = 0; i < AES_BLK_LENGTH; i++)
    // {
    //     printf("0x%02x ", buffer[i]);
    // }
    delete [] temp_buf;
}

// Function for calculating Galois Multiplication
uint8_t aes_galoi_mult(uint8_t num, uint8_t mult)
{
    uint8_t temp;

    if(mult == 0x03)
    {
        temp = aes_galoi_mult(num, 2) ^ num;
    }
    else if (mult == 0x02)
    {
        if((num & 0x80) == 0x80)
        {
            temp = (num << 1) ^ 0x1B;
        }
        else
        {
            temp = (num << 1) ^ 0x00;
        }
    }
    else
    {
        temp = num;
    }

    // printf("\n0x%02x 0x%02x 0x%02x \n", temp, num, mult);
    return temp;
}