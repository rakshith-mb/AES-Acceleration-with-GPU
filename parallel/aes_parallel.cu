/******************************************************************************
 * File Name    - aes_parallel.cu
 * 
 * Description  - This cu file contains the function definitions of all aes 
 *                helper functions
 ******************************************************************************/
#include "string.h"
#include <cuda.h>

#include "aes_parallel.cuh"

/*******************************************************************************
* Global constants
*******************************************************************************/

static const uint8_t sbox[SBOX_LENGTH] = {
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

// Combined buffer to save shift row constants and matrix of mix column step
static const int8_t comb_const[32] = {0, 12, 8, 4, 0, -4, 8, 4, 0, -4, -8, 4, 0, -4, -8, -12, 2, 3, 1, 1, 1, 2, 3, 1, 1, 1, 2, 3, 3, 1, 1, 2};

// Ref - https://stackoverflow.com/questions/14038589/what-is-the-canonical-way-to-check-for-errors-using-the-cuda-runtime-api
#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

// Helper function to get the S-Box value
uint8_t aes_sbox_get_val(uint8_t byte_val)
{
    return sbox[byte_val];
}

// Function to initialize the AES config structure
void aes_init(aes_struct* aes_config_struct)
{
    aes_config_struct->aes_mode = AES_MODE;
    aes_config_struct->aes_key_length = AES_KEY_SIZE;

    if(aes_config_struct->aes_key_length == AES128_KEY_SIZE*8)
    {
        aes_config_struct->round_key = new uint8_t[AES128_ROUND_KEY_LENGTH];
        aes_config_struct->round_key_length = AES128_ROUND_KEY_LENGTH;
    }
    else
    {
        aes_config_struct->round_key = new uint8_t[AES256_ROUND_KEY_LENGTH];
        aes_config_struct->round_key_length = AES256_ROUND_KEY_LENGTH;
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
        aes_encrypt_ctr(aes_config_struct);
    }
}

// Function to encrypt the buffer in CTR mode
void aes_encrypt_ctr(aes_struct* aes_config_struct)
{
    // Calculate the number of blocks needed
    int block_count = (aes_config_struct->plain_text_length + THREADS_PER_BLOCK - 1)/THREADS_PER_BLOCK;

    // Calculate the size required for the shared memory
    int smem_size = sizeof(uint8_t) * (SBOX_LENGTH + 2*THREADS_PER_BLOCK + AES256_ROUND_KEY_LENGTH) + sizeof(int8_t) * 32;

    uint8_t num_rounds;

    if(aes_config_struct->aes_key_length == AES128_KEY_SIZE*8)
    {
        num_rounds = 11;
    }
    else
    {
        num_rounds = 15;
    }

    uint8_t* plain_text_buf = aes_config_struct->plain_text;

    // IV is encrypted in CTR mode
    uint8_t* ctr_buf = new uint8_t[aes_config_struct->plain_text_length];

    for(int i = 0; i < aes_config_struct->plain_text_length; i++)
    {
        // Copy the IV to the buffer. IV is incremented for each block
        if((i % AES_BLK_LENGTH == 15) && (i != 15))
        {
            ctr_buf[i] = ctr_buf[i - AES_BLK_LENGTH] + 1;
        }
        else
        {
            ctr_buf[i] = aes_config_struct->counter[i % 16];
        }
    }

    // Reuse of the same code used for ECB mode
    aes_config_struct->plain_text = ctr_buf;

    uint8_t *dev_sbox_arr, *dev_round_key, *dev_plain_text, *dev_cipher_text;
    int8_t *dev_comb_arr;
    cudaMalloc((void**)&dev_sbox_arr, sizeof(uint8_t) * SBOX_LENGTH);
    cudaMalloc((void**)&dev_round_key, sizeof(uint8_t) * aes_config_struct->round_key_length);
    cudaMalloc((void**)&dev_plain_text, sizeof(uint8_t) * aes_config_struct->plain_text_length);
    cudaMalloc((void**)&dev_cipher_text, sizeof(uint8_t) * aes_config_struct->plain_text_length);
    cudaMalloc((void**)&dev_comb_arr, sizeof(int8_t) * 32);

    cudaMemcpy(dev_sbox_arr, sbox, sizeof(uint8_t) * SBOX_LENGTH, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_round_key, (aes_config_struct->round_key), sizeof(uint8_t) * aes_config_struct->round_key_length, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_plain_text, (aes_config_struct->plain_text), sizeof(uint8_t) * aes_config_struct->plain_text_length, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_comb_arr, comb_const, sizeof(int8_t) * 32, cudaMemcpyHostToDevice);

    cudaMemset(dev_cipher_text, 0, (sizeof(uint8_t) * aes_config_struct->plain_text_length));

    aes_ecb_gpu_encryption_kernel<<<block_count, THREADS_PER_BLOCK, smem_size>>>(dev_sbox_arr, dev_round_key, aes_config_struct->round_key_length, dev_plain_text, aes_config_struct->plain_text_length, num_rounds, dev_comb_arr, dev_cipher_text);

    cudaDeviceSynchronize();

    cudaMemcpy(aes_config_struct->cipher_text, dev_cipher_text, sizeof(uint8_t) * aes_config_struct->plain_text_length, cudaMemcpyDeviceToHost);

#if DEBUG
    printf("\nPrinting encrypted IV...\n");
    for(int i = 0; i < aes_config_struct->plain_text_length; i++)
    {
        printf("0x%02x ", aes_config_struct->cipher_text[i]);
    }
#endif
    
    // XOR the encrypted IV with the plain text 
    for(int i = 0; i < aes_config_struct->plain_text_length; i++)
    {
        aes_config_struct->cipher_text[i] = aes_config_struct->cipher_text[i] ^ plain_text_buf[i];
    }

    delete [] ctr_buf;
    cudaFree(dev_sbox_arr);
    cudaFree(dev_round_key);
    cudaFree(dev_plain_text);
    cudaFree(dev_cipher_text);
    cudaFree(dev_comb_arr);
}

// Function to encrypt the buffer in ECB mode
void aes_encrypt_ecb(aes_struct* aes_config_struct)
{
    // Calculate the number of blocks needed
    int block_count = (aes_config_struct->plain_text_length + THREADS_PER_BLOCK - 1)/THREADS_PER_BLOCK;

    // Calculate the size required for the shared memory
    int smem_size = sizeof(uint8_t) * (SBOX_LENGTH + 2*THREADS_PER_BLOCK + AES256_ROUND_KEY_LENGTH) + sizeof(int8_t) * 32;

    uint8_t num_rounds;

    if(aes_config_struct->aes_key_length == AES128_KEY_SIZE*8)
    {
        num_rounds = 11;
    }
    else
    {
        num_rounds = 15;
    }

    // Buffer for device arrays
    uint8_t *dev_sbox_arr, *dev_round_key, *dev_plain_text, *dev_cipher_text;
    int8_t *dev_comb_arr;

    // Allocate memory to the device buffers
    cudaMalloc((void**)&dev_sbox_arr, sizeof(uint8_t) * SBOX_LENGTH);
    cudaMalloc((void**)&dev_round_key, sizeof(uint8_t) * aes_config_struct->round_key_length);
    cudaMalloc((void**)&dev_plain_text, sizeof(uint8_t) * aes_config_struct->plain_text_length);
    cudaMalloc((void**)&dev_cipher_text, sizeof(uint8_t) * aes_config_struct->plain_text_length);
    cudaMalloc((void**)&dev_comb_arr, sizeof(int8_t) * 32);

    // Copy the values into device arrays
    cudaMemcpy(dev_sbox_arr, sbox, sizeof(uint8_t) * SBOX_LENGTH, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_round_key, (aes_config_struct->round_key), sizeof(uint8_t) * aes_config_struct->round_key_length, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_plain_text, (aes_config_struct->plain_text), sizeof(uint8_t) * aes_config_struct->plain_text_length, cudaMemcpyHostToDevice);
    cudaMemcpy(dev_comb_arr, comb_const, sizeof(int8_t) * 32, cudaMemcpyHostToDevice);

    // Set the cipher buffer to 0
    cudaMemset(dev_cipher_text, 0, (sizeof(uint8_t) * aes_config_struct->plain_text_length));

    // Call the kernel function
    aes_ecb_gpu_encryption_kernel<<<block_count, THREADS_PER_BLOCK, smem_size>>>(dev_sbox_arr, dev_round_key, aes_config_struct->round_key_length, dev_plain_text, aes_config_struct->plain_text_length, num_rounds, dev_comb_arr, dev_cipher_text);

    cudaDeviceSynchronize();

    // Copy the calculated cipher from the device array to host
    cudaMemcpy(aes_config_struct->cipher_text, dev_cipher_text, sizeof(uint8_t) * aes_config_struct->plain_text_length, cudaMemcpyDeviceToHost);
    
    cudaFree(dev_sbox_arr);
    cudaFree(dev_round_key);
    cudaFree(dev_plain_text);
    cudaFree(dev_cipher_text);
    cudaFree(dev_comb_arr);
}

__device__ inline uint8_t aes_galoi_mult(uint8_t num, uint8_t mult)
{
    // Calculate the Galois product
    return (mult == 0x03) ? (((num & 0x80) ? (num << 1) ^ 0x1B : (num << 1)) ^ num) : ((mult == 0x02) ? ((num & 0x80) ? (num << 1) ^ 0x1B : (num << 1)) : num);
}

// Kernel Function
__global__ void aes_ecb_gpu_encryption_kernel(const uint8_t* sbox_arr, uint8_t* round_key_arr, uint8_t round_key_length, uint8_t* plain_text_arr, int plain_text_length, uint8_t num_rounds, int8_t* comb_arr, uint8_t* cipher_text_arr)
{
    // Dynamic shared memory allocation
    extern __shared__ uint8_t smem[];

    // Calculate the smem size
    const int smem_size = sizeof(uint8_t) * (SBOX_LENGTH + 2*THREADS_PER_BLOCK + AES256_ROUND_KEY_LENGTH) + sizeof(int8_t) * 32;

    // Calculate thread_count
    uint32_t thread_count = threadIdx.x + blockIdx.x * blockDim.x;

    // Calculate for easier computation
    uint32_t temp = thread_count * 2;

    uint8_t state_element, curr_round = 0;
    uint32_t index;

#if DEBUG
    if(threadIdx.x == 0)
    {
        printf("\nPrinting sbox values passed to device...\n");
        for(int i = 0; i < SBOX_LENGTH; i++)
        {
            printf("0x%02x ", sbox_arr[i]);
        }

        printf("\nPrinting round key values passed to device...\n");
        for(int i = 0; i < round_key_length; i++)
        {
            printf("0x%02x ", round_key_arr[i]);
        }

        printf("\nPrinting plain text values passed to device...\n");
        for(int i = 0; i < plain_text_length; i++)
        {
            printf("0x%02x ", plain_text_arr[i]);
        }
    }
#endif
    
    // Copy values from buffers to shared memory
    if(threadIdx.x < 128)
    {
        smem[threadIdx.x * 2] = sbox_arr[threadIdx.x * 2];
        smem[threadIdx.x * 2 + 1] = sbox_arr[threadIdx.x * 2 + 1];
    }
    else if((threadIdx.x >= 128) && (threadIdx.x < (248)) && ((threadIdx.x - 128) * 2 < round_key_length))
    {
        smem[threadIdx.x * 2] = round_key_arr[(threadIdx.x - 128) * 2];
        smem[threadIdx.x * 2 + 1] = round_key_arr[(threadIdx.x - 128) * 2 + 1];
    }
    else if((threadIdx.x >= 248) && (threadIdx.x < 256))
    {
        smem[1520 + (threadIdx.x - 248)*4] = comb_arr[(threadIdx.x - 248)*4];
        smem[1521 + (threadIdx.x - 248)*4] = comb_arr[(threadIdx.x - 248)*4 + 1];
        smem[1522 + (threadIdx.x - 248)*4] = comb_arr[(threadIdx.x - 248)*4 + 2];
        smem[1523 + (threadIdx.x - 248)*4] = comb_arr[(threadIdx.x - 248)*4 + 3];
    }
    else if((threadIdx.x >= 256) && ((threadIdx.x - 256) * 2 < plain_text_length))
    {
        smem[(threadIdx.x - 8) * 2] = plain_text_arr[(blockIdx.x * blockDim.x + (threadIdx.x - 256) * 2)];
        smem[(threadIdx.x - 8) * 2 + 1] = plain_text_arr[(blockIdx.x * blockDim.x + (threadIdx.x - 256) * 2) + 1];
    }

    // Allocate pointers in shared memory
    uint8_t* sbox = (uint8_t*)smem;
    uint8_t* round_key = (uint8_t*)&sbox[SBOX_LENGTH];
    uint8_t* plain_text = (uint8_t*)&smem[496];
    uint8_t* cipher_text = (uint8_t*)&smem[1008];
    int8_t* shift_row_const = (int8_t*)&smem[1520];
    uint8_t* mult_arr = (uint8_t*)&smem[1536];

    // Wait till all threads complete transfer to shared mem
    __syncthreads();

#if DEBUG
    if(thread_count == 0)
    {
        printf("\nSmem size = %d\n", smem_size);

        printf("\nPrinting sbox values in shared memory...\n");
        for(int i = 0; i < SBOX_LENGTH; i++)
        {
            printf("0x%02x ", sbox[i]);
        }

        printf("\nPrinting round key values in shared memory...\n");
        for(int i = 0; i < round_key_length; i++)
        {
            printf("0x%02x ", round_key[i]);
        }

        printf("\nPrinting plain text values in shared memory...\n");
        for(int i = 0; i < THREADS_PER_BLOCK; i++)
        {
            printf("%02x", plain_text[i]);
        }

        printf("\nPrinting shift row const values in shared memory...\n");
        for(int i = 0; i < 16; i++)
        {
            printf("%d ", shift_row_const[i]);
        }
        
        printf("\nPrinting mult arr values in shared memory...\n");
        for(int i = 0; i < 16; i++)
        {
            printf("0x%02x ", mult_arr[i]);
        }
    }
#endif

    if(thread_count < plain_text_length)
    {
        // Ensure that the threads are within bounds
        index = thread_count;
        temp = thread_count % 16;

        // Add round key
        state_element = plain_text[threadIdx.x] ^ round_key[temp];
        ++curr_round;

        for(; curr_round < num_rounds - 1; curr_round++)
        {
            // Substitute S-box matrix
            state_element = sbox[state_element];

            // Shift rows. Change index values and move the elements
            index = (index + shift_row_const[temp]) % 512;
            cipher_text[index] = state_element;

            // Wait for all threads to complete
            __syncwarp();
            
            index = threadIdx.x;

            // Mix Columns step 
            state_element = aes_galoi_mult(cipher_text[(index / 4) * 4], mult_arr[(index % 4) * 4]) ^ aes_galoi_mult(cipher_text[(index / 4) * 4 + 1], mult_arr[(index % 4) * 4 + 1]) ^ aes_galoi_mult(cipher_text[(index / 4) * 4 + 2], mult_arr[(index % 4) * 4 + 2]) ^ aes_galoi_mult(cipher_text[(index / 4) * 4 + 3], mult_arr[(index % 4) * 4 + 3]);
            
            // Add round key step
            state_element = state_element ^ round_key[curr_round*AES_BLK_LENGTH + (temp)];

            // Copy the state element back to shared memory buffer
            cipher_text[index] = state_element;
        }

        // Substitution step
        state_element = sbox[state_element];

        index = threadIdx.x;

        // Shift row step. Change index values instead of moving the elements
        index = (index + shift_row_const[temp]) % 512;

        // Add round key
        state_element = state_element ^ round_key[(curr_round)*AES_BLK_LENGTH + (temp) + shift_row_const[temp]];
        
        // Copy element back to the shared memory buffer
        cipher_text[index] = state_element;

        // Finally copy from shared memory to the device buffer
        cipher_text_arr[thread_count] = cipher_text[threadIdx.x];
    }
}
