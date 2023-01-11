/******************************************************************************
 * File Name    - key_helper.cu
 * 
 * Description  - This cpp file contains the function definitions of all key 
 *                helper functions
 ******************************************************************************/

#include "key_helper.cuh"
#include "aes_parallel.cuh"
#include "omp.h"

/*******************************************************************************
* Global constants
*******************************************************************************/
// Round constant buffer
static const uint8_t round_const[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

// Function for key expansion
void key_helper_create_round_keys(uint8_t aes_mode, uint8_t aes_key_length, const uint8_t* key, uint8_t* round_key)
{
    uint8_t i = 0;
    uint8_t num_rounds;

    if(aes_key_length == AES128_KEY_SIZE*8)
    {
        num_rounds = 10;
    }
    else
    {
        num_rounds = 14;
    }

#if (ENABLE_NAIVE | !ENABLE_OPENMP)
    // First block of round key is same as the original key
    for(; i < 16; i++)
    {
        round_key[i] = key[i];
    }

    for(; num_rounds > 0; num_rounds--)
    {
        // Generate subsequent round key blocks
        generate_round_key(round_key, i);
        i+=16;
    }
#endif

#if ENABLE_OPENMP
    #pragma omp parallel sections num_threads(4)
    {
        /* OpenMP implementation considers one column at a time which has lesser 
         * dependencies compared to row wise implementation. Each core 
         * handles 1 column.
         */
        #pragma omp section
        {
            key_helper_generate_round_key_per_core(key, round_key, 3, num_rounds);
        }

        #pragma omp section
        {
            key_helper_generate_round_key_per_core(key, round_key, 0, num_rounds);
        }
        
        #pragma omp section
        {
            key_helper_generate_round_key_per_core(key, round_key, 1, num_rounds);
        }

        #pragma omp section
        {
            key_helper_generate_round_key_per_core(key, round_key, 2, num_rounds);
        }
    }
#endif
}

#if ENABLE_OPENMP
void key_helper_generate_round_key_per_core(const uint8_t* key, uint8_t* round_key, uint8_t init_index, uint8_t num_rounds)
{
    uint8_t prev_index;
    uint8_t curr_index = init_index;

    // Copy the row corresponding to the index
    round_key[curr_index] = key[curr_index];
    round_key[curr_index + 4] = key[curr_index + 4];
    round_key[curr_index + 8] = key[curr_index + 8];

    curr_index = curr_index + 12;
    round_key[curr_index] = key[curr_index];

    for(uint8_t i = 0; i < num_rounds; i++)
    {
        #pragma omp barrier
        uint8_t temp;

        prev_index = curr_index;
        
        // Shift row step. Here instead of shifting the elements, index is shifted
        curr_index = (curr_index % 16 == 12) ? (curr_index + 3) : (curr_index - 1);

        // Substitute step. Value is fetched from the S-Box and XORed with round constant simultaneously
        temp = (curr_index % 16 == 12) ? (aes_sbox_get_val(round_key[prev_index]) ^ round_const[i]) : (aes_sbox_get_val(round_key[prev_index]));

        curr_index += 4;
        // Substituted value is XORed with the previous value in the same index
        round_key[curr_index] = round_key[curr_index - 16] ^ temp;

        curr_index +=4;
        // Previous value in the same index is XORed with the calculated previous row value
        round_key[curr_index] = round_key[curr_index - 4] ^ round_key[curr_index - 16];

        curr_index += 4;
        round_key[curr_index] = round_key[curr_index - 4] ^ round_key[curr_index - 16];

        curr_index +=4;
        round_key[curr_index] = round_key[curr_index - 4] ^ round_key[curr_index - 16];

        /* This is repeated num_rounds times. The dependency on previous rounds in 
         * AES key expansion step is completely removed.
         */
    }
}
#endif

#if (ENABLE_NAIVE | !ENABLE_OPENMP)
void generate_round_key(uint8_t* round_key, int offset)
{
    // Calculates one block of AES key
    uint8_t temp_col[4];
    uint8_t temp_byte;
    int prev_key_offset = offset - 16;

    int num_rounds_compl = offset/16;

    // Initial value - the last column of the previous block (column wise)
    temp_col[0] = round_key[prev_key_offset + 12];
    temp_col[1] = round_key[prev_key_offset + 13];
    temp_col[2] = round_key[prev_key_offset + 14];
    temp_col[3] = round_key[prev_key_offset + 15];

    // Shift column by 1
    temp_byte = temp_col[0];
    temp_col[0] = temp_col[1];
    temp_col[1] = temp_col[2];
    temp_col[2] = temp_col[3];
    temp_col[3] = temp_byte;

    // Substitute step
    temp_col[0] = aes_sbox_get_val(temp_col[0]);
    temp_col[1] = aes_sbox_get_val(temp_col[1]);
    temp_col[2] = aes_sbox_get_val(temp_col[2]);
    temp_col[3] = aes_sbox_get_val(temp_col[3]);

    // XOR the first element with round constant
    temp_col[0] ^= round_const[num_rounds_compl - 1];

    // XOR the calculated column with previous column in the same index
    round_key[offset] = round_key[prev_key_offset] ^ temp_col[0];
    round_key[offset + 1] = round_key[prev_key_offset + 1] ^ temp_col[1];
    round_key[offset + 2] = round_key[prev_key_offset + 2] ^ temp_col[2];
    round_key[offset + 3] = round_key[prev_key_offset + 3] ^ temp_col[3];

    for(int j = 4; j < 16; j+=4)
    {
        /* The subsequent columns are obtained by XOR of elements of column-1 calculated 
         * previously and the elemnts of the same column of previous block
         */
        round_key[offset + j] = round_key[prev_key_offset + j] ^ round_key[offset + j - 4];
        round_key[offset + j + 1] = round_key[prev_key_offset + j + 1] ^ round_key[offset + j - 4 + 1];
        round_key[offset + j + 2] = round_key[prev_key_offset + j + 2] ^ round_key[offset + j - 4 + 2];
        round_key[offset + j + 3] = round_key[prev_key_offset + j + 3] ^ round_key[offset + j - 4 + 3];
    }
}
#endif
