/**
 * @file sd_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SD card driver test interface 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _SD_CARD_TEST_H_
#define _SD_CARD_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief SD card setup code 
 */
void sd_test_init(void); 


/**
 * @brief SD card test code 
 */
void sd_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _SD_CARD_TEST_H_ 
