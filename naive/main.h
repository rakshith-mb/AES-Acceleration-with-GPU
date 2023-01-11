/******************************************************************************
 * File Name    - main.h
 * 
 * Description  - This is the header file for the main code of the naive AES 
 *                implementation
 ******************************************************************************/

#ifndef SOURCE_MAIN_H_
#define SOURCE_MAIN_H_

#include <iostream>
#include <cstdlib>
#include <cstdint>

/*******************************************************************************
* Global constants
*******************************************************************************/
#define DEBUG                       0
#define DISPLAY_INPUTS              1
#define COPYABLE_FORMAT             1

#define USE_DEFAULT_INPUTS          1

#define TIME_NAIVE                  1

#define AES_MODE                    AES_ECB
#define AES_KEY_SIZE                128
#define AES_KEY_SIZE_BYTES          AES_KEY_SIZE/8

#define AES_ECB                     0x00
#define AES_CTR                     0x01

#define AES_BLK_LENGTH              16

#endif /* SOURCE_MAIN_H_ */

/* [] END OF FILE */