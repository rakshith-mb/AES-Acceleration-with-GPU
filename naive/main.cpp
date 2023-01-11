/******************************************************************************
 * File Name    - main.cpp
 * 
 * Description  - This is the source code for the ME 759 final project - AES 
 *                acceleration using GPU. This file is the main code which 
 *                demonstrates the use of naive AES implementation
 ******************************************************************************/
#include <random>
#include <assert.h>
#include <chrono>

#include "main.h"
#include "aes_naive.h"
#include "key_helper.h"

/*******************************************************************************
* Global constants
*******************************************************************************/

/******************************************************************************
 * Default key - 
 * 2b7e151628aed2a6abf7158809cf4f3c
 *****************************************************************************/
const uint8_t default_key[AES_KEY_SIZE_BYTES] = {
    0x2b, 0x7e, 0x15, 0x16, 
    0x28, 0xae, 0xd2, 0xa6, 
    0xab, 0xf7, 0x15, 0x88, 
    0x09, 0xcf, 0x4f, 0x3c
    };

/******************************************************************************
 * Default plain text - 
 * 73e309bfebec93dc306dcfdcb26be593b7148985780f754622e7949ec91582b1c74339bae316d07496dbfd6bdb0eddad28daa669fc2cad25d7dbe4fd02eb32ecb510ea6f615e2aef5c50c61c1b80dee389d477df4cc334d1e3d00b8f235ee6e9a0f348a7c54c05201129a81d796241af46bececcc841f61f0bff3eaddc90fc3a4d377358921ff01404790d470daa38671853bb9027e4a0c2cb01f9d86f31456b3c0700c2b69f68523ca3f61fd83e5f776245844e714f30cc9c287b9a3a6e291c8da97de9c481b69f313413f9d32976bc1dfd33839730e5edeccd7e5f1b3a960ef6028342414dcd9e0170ad372bf30c373cb7944648856dc057008fc6826b4e39ee9a5c85e5336bf0438ef547659a547e78d0ac58707f4882bc8e37951edabb48d8ffe9f8794cf9f47cf5969a004263e3a9cd1c8760484d1aa7735b992359106c576ba04740e488abd5c85b0d14ed77e885698ca46b4501a8f6abd4a5d60ca62c0fc21156b8e5b0311da1c739a0079f790e4b269ef95c848c54d74afbfb8bb33ce1d2f10b8b26e6ed9ed3418e464b431cf41ae44c9b6b2d8e14c3e1397cbe8348e98569acf130b9afd61568f046c62c32ba6c28424835c07324ef2507de3f230c4790a7d2bd699e156f6314af82b886db49babab390eda854849a0fb53f1f13c7b75b1807fc3f96fd881ae1cf3528a1401ed6bd3036e65d580551dc1fbfbc4455
 *****************************************************************************/
uint8_t default_plain_text[512] = {
    0x73, 0xe3, 0x09, 0xbf, 0xeb, 0xec, 0x93, 0xdc, 0x30, 0x6d, 0xcf, 0xdc, 0xb2, 0x6b, 0xe5, 0x93, 0xb7, 0x14, 
    0x89, 0x85, 0x78, 0x0f, 0x75, 0x46, 0x22, 0xe7, 0x94, 0x9e, 0xc9, 0x15, 0x82, 0xb1, 0xc7, 0x43, 0x39, 0xba, 
    0xe3, 0x16, 0xd0, 0x74, 0x96, 0xdb, 0xfd, 0x6b, 0xdb, 0x0e, 0xdd, 0xad, 0x28, 0xda, 0xa6, 0x69, 0xfc, 0x2c, 
    0xad, 0x25, 0xd7, 0xdb, 0xe4, 0xfd, 0x02, 0xeb, 0x32, 0xec, 0xb5, 0x10, 0xea, 0x6f, 0x61, 0x5e, 0x2a, 0xef, 
    0x5c, 0x50, 0xc6, 0x1c, 0x1b, 0x80, 0xde, 0xe3, 0x89, 0xd4, 0x77, 0xdf, 0x4c, 0xc3, 0x34, 0xd1, 0xe3, 0xd0, 
    0x0b, 0x8f, 0x23, 0x5e, 0xe6, 0xe9, 0xa0, 0xf3, 0x48, 0xa7, 0xc5, 0x4c, 0x05, 0x20, 0x11, 0x29, 0xa8, 0x1d, 
    0x79, 0x62, 0x41, 0xaf, 0x46, 0xbe, 0xce, 0xcc, 0xc8, 0x41, 0xf6, 0x1f, 0x0b, 0xff, 0x3e, 0xad, 0xdc, 0x90, 
    0xfc, 0x3a, 0x4d, 0x37, 0x73, 0x58, 0x92, 0x1f, 0xf0, 0x14, 0x04, 0x79, 0x0d, 0x47, 0x0d, 0xaa, 0x38, 0x67, 
    0x18, 0x53, 0xbb, 0x90, 0x27, 0xe4, 0xa0, 0xc2, 0xcb, 0x01, 0xf9, 0xd8, 0x6f, 0x31, 0x45, 0x6b, 0x3c, 0x07, 
    0x00, 0xc2, 0xb6, 0x9f, 0x68, 0x52, 0x3c, 0xa3, 0xf6, 0x1f, 0xd8, 0x3e, 0x5f, 0x77, 0x62, 0x45, 0x84, 0x4e, 
    0x71, 0x4f, 0x30, 0xcc, 0x9c, 0x28, 0x7b, 0x9a, 0x3a, 0x6e, 0x29, 0x1c, 0x8d, 0xa9, 0x7d, 0xe9, 0xc4, 0x81, 
    0xb6, 0x9f, 0x31, 0x34, 0x13, 0xf9, 0xd3, 0x29, 0x76, 0xbc, 0x1d, 0xfd, 0x33, 0x83, 0x97, 0x30, 0xe5, 0xed, 
    0xec, 0xcd, 0x7e, 0x5f, 0x1b, 0x3a, 0x96, 0x0e, 0xf6, 0x02, 0x83, 0x42, 0x41, 0x4d, 0xcd, 0x9e, 0x01, 0x70, 
    0xad, 0x37, 0x2b, 0xf3, 0x0c, 0x37, 0x3c, 0xb7, 0x94, 0x46, 0x48, 0x85, 0x6d, 0xc0, 0x57, 0x00, 0x8f, 0xc6, 
    0x82, 0x6b, 0x4e, 0x39, 0xee, 0x9a, 0x5c, 0x85, 0xe5, 0x33, 0x6b, 0xf0, 0x43, 0x8e, 0xf5, 0x47, 0x65, 0x9a, 
    0x54, 0x7e, 0x78, 0xd0, 0xac, 0x58, 0x70, 0x7f, 0x48, 0x82, 0xbc, 0x8e, 0x37, 0x95, 0x1e, 0xda, 0xbb, 0x48, 
    0xd8, 0xff, 0xe9, 0xf8, 0x79, 0x4c, 0xf9, 0xf4, 0x7c, 0xf5, 0x96, 0x9a, 0x00, 0x42, 0x63, 0xe3, 0xa9, 0xcd, 
    0x1c, 0x87, 0x60, 0x48, 0x4d, 0x1a, 0xa7, 0x73, 0x5b, 0x99, 0x23, 0x59, 0x10, 0x6c, 0x57, 0x6b, 0xa0, 0x47, 
    0x40, 0xe4, 0x88, 0xab, 0xd5, 0xc8, 0x5b, 0x0d, 0x14, 0xed, 0x77, 0xe8, 0x85, 0x69, 0x8c, 0xa4, 0x6b, 0x45, 
    0x01, 0xa8, 0xf6, 0xab, 0xd4, 0xa5, 0xd6, 0x0c, 0xa6, 0x2c, 0x0f, 0xc2, 0x11, 0x56, 0xb8, 0xe5, 0xb0, 0x31, 
    0x1d, 0xa1, 0xc7, 0x39, 0xa0, 0x07, 0x9f, 0x79, 0x0e, 0x4b, 0x26, 0x9e, 0xf9, 0x5c, 0x84, 0x8c, 0x54, 0xd7, 
    0x4a, 0xfb, 0xfb, 0x8b, 0xb3, 0x3c, 0xe1, 0xd2, 0xf1, 0x0b, 0x8b, 0x26, 0xe6, 0xed, 0x9e, 0xd3, 0x41, 0x8e, 
    0x46, 0x4b, 0x43, 0x1c, 0xf4, 0x1a, 0xe4, 0x4c, 0x9b, 0x6b, 0x2d, 0x8e, 0x14, 0xc3, 0xe1, 0x39, 0x7c, 0xbe, 
    0x83, 0x48, 0xe9, 0x85, 0x69, 0xac, 0xf1, 0x30, 0xb9, 0xaf, 0xd6, 0x15, 0x68, 0xf0, 0x46, 0xc6, 0x2c, 0x32, 
    0xba, 0x6c, 0x28, 0x42, 0x48, 0x35, 0xc0, 0x73, 0x24, 0xef, 0x25, 0x07, 0xde, 0x3f, 0x23, 0x0c, 0x47, 0x90, 
    0xa7, 0xd2, 0xbd, 0x69, 0x9e, 0x15, 0x6f, 0x63, 0x14, 0xaf, 0x82, 0xb8, 0x86, 0xdb, 0x49, 0xba, 0xba, 0xb3, 
    0x90, 0xed, 0xa8, 0x54, 0x84, 0x9a, 0x0f, 0xb5, 0x3f, 0x1f, 0x13, 0xc7, 0xb7, 0x5b, 0x18, 0x07, 0xfc, 0x3f, 
    0x96, 0xfd, 0x88, 0x1a, 0xe1, 0xcf, 0x35, 0x28, 0xa1, 0x40, 0x1e, 0xd6, 0xbd, 0x30, 0x36, 0xe6, 0x5d, 0x58, 
    0x05, 0x51, 0xdc, 0x1f, 0xbf, 0xbc, 0x44, 0x55
    };

/******************************************************************************
 * Default cipher text - 
 * 00de55d6b8a83dbfa251a5aa83c1ba50c3e8d641fbd800f26be5428170381db08bece48ce9320c995c07a51ff52fc13feec7b531427949e838aefdab7a37b92cafab5ad150d8ee45be82f1c914d5b0e668cd70fdc83af35b7cb0765041b35ddbabb60c4b8f3abef881f0af519244a3909092f7d801daaf6b35e122f4f796539833c604fe7c8e89f3ed76117b2bf747984a921131ded2e9dd8422c17d8b96a2ae3560a317e6f91f07b6d58ca8128ab89d2b0ac5e22e0c767be6ea0641086e100a3899318ad681aa8365c444df73577e504f781eb81a678c7d4d68319c5970cd891834d63fe396b672e7f3f2d3096b740f40b1d36867553c94e676a27553416cac9f0fa438eb5c0289619ca901065eb7352f20a1c47a2913de3c3b0fa7c314303cbabdcd0a15a9b620027d942a376e44fc4da4a64d26d2ef9b9ff28fda8e1b15e9018badb524dd6b0386b52c42793930ccfa12e12dc9606ec21d8bde907f420a545a2a6e258a74665da41d611614ae55d8d55e6407035ddc47bdc7693a5ef36593ded5abfa2723a4224a3474788debfed4df2d21529b1efa31916061710883ddb6f7c2bea1de6dd546d81bb79b0811042c6c28692deede4b5d86b9665ea0ef2acdbc681b3afaa8be309bf4b3924daf0a487077414756586448eb16685f6152bcfaecaac6425faf7cca688ac6edb9ffc064fb34fbac66ccd90fbfce6e637dc6f194
 *****************************************************************************/

using namespace std;

int main(int argc, char* argv[])
{
    // Structure to store all AES configuration
    aes_struct encrypt_struct;

#if USE_DEFAULT_INPUTS
    int plain_text_size = sizeof(default_plain_text)/sizeof(uint8_t);

    encrypt_struct.plain_text = default_plain_text;
    encrypt_struct.plain_text_length = plain_text_size;
    encrypt_struct.key = default_key;
#else
    // Fetch the value of n
    int plain_text_size = atoi(argv[1]);
    uint8_t* plain_text = new uint8_t[plain_text_size];
    uint8_t* key = new uint8_t[AES_KEY_SIZE_BYTES];
    
    // Generating random numbers for array with the arbitrary range
    std::random_device entropy_source_input;
    std::mt19937 generator_input(entropy_source_input()); 
    std::uniform_int_distribution<int> dist_plain_text(0, 255);
    std::uniform_int_distribution<int> dist_key(0, 255);

    for(int i = 0; i < plain_text_size; i++)
    {
        plain_text[i] = dist_plain_text(generator_input);

        if(i < AES_KEY_SIZE_BYTES)
        {
            key[i] = dist_key(generator_input);
        }
    }
    encrypt_struct.plain_text = plain_text;
    encrypt_struct.plain_text_length = plain_text_size;
    encrypt_struct.key = key;
#endif

    printf("************************************************************\n");
    printf("*     AES Acceleration with GPU - Naive implemntation      *\n");
    printf("************************************************************\n");

    // ToDo: Accept AES mode
    if(plain_text_size % 16 != 0)
    {
        printf("ERROR: Buffer length is not a multiple of 128 bits\n");
        assert(0);
    }

    // Initialize the AES structure
    aes_init(&encrypt_struct);

    // Buffer to store calculated cipher 
    uint8_t* cipher = new uint8_t[encrypt_struct.plain_text_length];
    encrypt_struct.cipher_text = cipher;

    if(encrypt_struct.aes_mode == AES_CTR)
    {
        // Generating random numbers for the initialization vector (IV) counter
        std::random_device entropy_source;
        std::mt19937 generator(entropy_source()); 
        std::uniform_int_distribution<int> dist(0, 255);

        uint8_t counter[AES_BLK_LENGTH];

        for(int i = 0; i < AES_BLK_LENGTH; i++)
        {
            counter[i] = dist(generator);
        }

        encrypt_struct.counter = counter;
    }

#if TIME_NAIVE
    std::chrono::high_resolution_clock::time_point start0;
    std::chrono::high_resolution_clock::time_point end0;
    std::chrono::duration<double, std::milli> duration_sec0;
#endif

#if TIME_NAIVE
    // Get start time
    start0 = std::chrono::high_resolution_clock::now();
#endif

    // Function for key expansion
    key_helper_create_round_keys(encrypt_struct.aes_mode, encrypt_struct.aes_key_length, encrypt_struct.key, encrypt_struct.round_key);

#if TIME_NAIVE
    // Get end time
    end0 = std::chrono::high_resolution_clock::now();
    duration_sec0 = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end0 - start0);
    // Print time taken in ms
    printf("\nTime taken for keygen using naive impl - %lf\n", duration_sec0.count());
#endif

#if DEBUG | DISPLAY_INPUTS
    #if COPYABLE_FORMAT
        printf("\nPrinting plain text values:\n");
        for(int i = 0; i < plain_text_size; i++)
        {
            printf("%02x", encrypt_struct.plain_text[i]);
        }
        printf("\n");

        printf("\nPrinting key values:\n");
        for(int i = 0; i < AES_KEY_SIZE_BYTES; i++)
        {
            printf("%02x", encrypt_struct.key[i]);
        }
        printf("\n");
    #else
        printf("\nPrinting plain text values:\n");
        for(int i = 0; i < plain_text_size; i++)
        {
            printf("0x%02x ", encrypt_struct.plain_text[i]);
        }
        printf("\n");

        printf("\nPrinting key values:\n");
        for(int i = 0; i < AES_KEY_SIZE_BYTES; i++)
        {
            printf("0x%02x ", encrypt_struct.key[i]);
        }
        printf("\n");
    #endif
#endif

#if DEBUG
    cout << "Printing round key values...\n";
    for(int i = 0; i < 176; i++)
    {
        printf("0x%02x ", encrypt_struct.round_key[i]);
    }
#endif

#if TIME_NAIVE
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;
    std::chrono::duration<double, std::milli> duration_sec;
#endif

#if TIME_NAIVE
    // Get start time
    start = std::chrono::high_resolution_clock::now();
#endif

    // Function call for AES encryption
    aes_encrypt_buffer(&encrypt_struct);

#if TIME_NAIVE
    // Get end time
    end = std::chrono::high_resolution_clock::now();
    duration_sec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);
    printf("\nTime taken for AES encryption using naive impl - %lf\n", duration_sec.count());
#endif

#if DEBUG | DISPLAY_INPUTS
    #if COPYABLE_FORMAT
        printf("\nPrinting cipher text values:\n");
        for(int i = 0; i < plain_text_size; i++)
        {
            printf("%02x", encrypt_struct.cipher_text[i]);
        }
        printf("\n");
    #else
        printf("\nPrinting cipher text values:\n");
        for(int i = 0; i < plain_text_size; i++)
        {
            printf("0x%02x ", encrypt_struct.cipher_text[i]);
        }
        printf("\n");
    #endif
#endif

    // Deallocate memory
    delete [] cipher;
    delete [] encrypt_struct.round_key;
#if !USE_DEFAULT_INPUTS
    delete [] plain_text;
    delete [] key;
#endif
}

