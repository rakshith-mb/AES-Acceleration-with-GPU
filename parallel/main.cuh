/******************************************************************************
 * File Name    - main.cuh
 * 
 * Description  - This is the header file for the main code of the parallel AES 
 *                implementation
 ******************************************************************************/
#ifndef SOURCE_MAIN_CUH
#define SOURCE_MAIN_CUH

#include <iostream>
#include <cstdlib>
#include <cstdint>

/*******************************************************************************
* References
********************************************************************************
* - Code references are added as a comment over the code
* - For AES visualization - https://www.cryptool.org/en/ct2/
* - For the understanding of AES:
*       * https://en.wikipedia.org/wiki/Advanced_Encryption_Standard
*       * https://www.youtube.com/watch?v=h6wvqm0aXco
* - For mix columns step - https://www.angelfire.com/biz7/atleast/mix_columns.pdf
* - AES online calculators for analysis and verification
*       * ECB - http://aes.online-domain-tools.com/
*             - https://the-x.cn/en-US/cryptography/Aes.aspx
*       * CTR - https://cryptii.com/pipes/aes-encryption
 ******************************************************************************/

/*******************************************************************************
* Global constants
*******************************************************************************/
// Enables debug outputs like printing round keys
#define DEBUG                       0

// Enables printing of the inputs to the AES algorithm (key, plain text)
#define DISPLAY_INPUTS              1

/* Prints will be in the form of a bit stream instead of 0xbb so that it can be
 * directly copied for verification
 */ 
#define COPYABLE_FORMAT             1

// Use default inputs or randomize plain text and key
#define USE_DEFAULT_INPUTS          1

/* ENABLE_NAIVE  - Uses naive implementation for key generation
 * ENABLE_OPENMP - Uses OpenMP for key generation
 * ENABLE_CUDA   - Unused, reserved
 * 
 * Note - Remember to change taskrun.sh
 */
#define ENABLE_NAIVE                1
#define ENABLE_OPENMP               0
#define ENABLE_CUDA                 1

/* TIME_NAIVE  - Enable timing of naive key generation
 * TIME_OPENMP - Enable timing of OpenMP key generation
 * TIME_CUDA   - Enable timing of CUDA AES implementation
 */
#define TIME_NAIVE                  1
#define TIME_OPENMP                 0
#define TIME_CUDA                   1

// Changes the threads per block. Currently application tailored for 512 threads
// ToDo: Scale AES for variable threads
#define THREADS_PER_BLOCK           512

// Configure the AES mode. Supported values - AES_ECB, AES_CTR
#define AES_MODE                    AES_ECB
#define AES_ECB                     0x00
#define AES_CTR                     0x01

// Configure AES key size. Currently supports only 128
// ToDo: Scale AES for 256 bit keys
#define AES_KEY_SIZE                128

// For internal reference
#define AES_KEY_SIZE_BYTES          AES_KEY_SIZE/8

// Block length of AES
#define AES_BLK_LENGTH              16

#endif /* SOURCE_MAIN_CUH */

/* [] END OF FILE */
