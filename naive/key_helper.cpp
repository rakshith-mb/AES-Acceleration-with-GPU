/******************************************************************************
 * File Name    - key_helper.cpp
 * 
 * Description  - This cpp file contains the function definitions of all key 
 *                helper functions
 ******************************************************************************/

#include "key_helper.h"
#include "aes_naive.h"

// Function to calculate the round constant
uint8_t round_const(int num_rounds_compl)
{
    uint8_t prev_rc;

    if(num_rounds_compl == 1)
    {
        return 1;
    }
    else
    {
        prev_rc = round_const(num_rounds_compl - 1);
        if(prev_rc < 0x80)
            return 2*prev_rc;
        else
            return (2*prev_rc)^0x11B;
    }
}

// Function to generate one block of round key
void generate_round_key(uint8_t* round_key, int offset)
{
    uint8_t temp_col[4];
    uint8_t temp_byte;
    int prev_key_offset = offset - 16;

    int num_rounds_compl = offset/16;

    temp_col[0] = round_key[prev_key_offset + 12];
    temp_col[1] = round_key[prev_key_offset + 13];
    temp_col[2] = round_key[prev_key_offset + 14];
    temp_col[3] = round_key[prev_key_offset + 15];

    temp_byte = temp_col[0];
    temp_col[0] = temp_col[1];
    temp_col[1] = temp_col[2];
    temp_col[2] = temp_col[3];
    temp_col[3] = temp_byte;

    temp_col[0] = aes_sbox_get_val(temp_col[0]);
    temp_col[1] = aes_sbox_get_val(temp_col[1]);
    temp_col[2] = aes_sbox_get_val(temp_col[2]);
    temp_col[3] = aes_sbox_get_val(temp_col[3]);

    temp_col[0] ^= round_const(num_rounds_compl);

    round_key[offset] = round_key[prev_key_offset] ^ temp_col[0];
    round_key[offset + 1] = round_key[prev_key_offset + 1] ^ temp_col[1];
    round_key[offset + 2] = round_key[prev_key_offset + 2] ^ temp_col[2];
    round_key[offset + 3] = round_key[prev_key_offset + 3] ^ temp_col[3];

    for(int j = 4; j < 16; j+=4)
    {
        round_key[offset + j] = round_key[prev_key_offset + j] ^ round_key[offset + j - 4];
        round_key[offset + j + 1] = round_key[prev_key_offset + j + 1] ^ round_key[offset + j - 4 + 1];
        round_key[offset + j + 2] = round_key[prev_key_offset + j + 2] ^ round_key[offset + j - 4 + 2];
        round_key[offset + j + 3] = round_key[prev_key_offset + j + 3] ^ round_key[offset + j - 4 + 3];
    }
}

// Function for key expansion
void key_helper_create_round_keys(uint8_t aes_mode, uint8_t aes_key_length, const uint8_t* key, uint8_t* round_key)
{
    int i = 0;
    int num_rounds;

    if(aes_key_length == AES128_KEY_SIZE*8)
    {
        num_rounds = 10;
    }
    else
    {
        num_rounds = 14;
    }

    for(; i < 16; i++)
    {
        round_key[i] = key[i];
    }

    for(; num_rounds > 0; num_rounds--)
    {
        generate_round_key(round_key, i);
        i+=16;
    }

}