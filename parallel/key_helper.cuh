/******************************************************************************
 * File Name    - key_helper.cuh
 * 
 * Description  - This is the header file for the key_helper code
 ******************************************************************************/

#ifndef SOURCE_KEY_HELPER_CUH
#define SOURCE_KEY_HELPER_CUH

#include "main.cuh"
/*******************************************************************************
* Function prototypes
*******************************************************************************/
void key_helper_create_round_keys(uint8_t aes_mode, uint8_t aes_key_length, const uint8_t* key, uint8_t* round_key);
void key_helper_generate_round_key_per_core(const uint8_t* key, uint8_t* round_key, uint8_t init_index, uint8_t num_rounds);
void generate_round_key(uint8_t* round_key, int offset);

#endif /* SOURCE_KEY_HELPER_CUH */

/* [] END OF FILE */
