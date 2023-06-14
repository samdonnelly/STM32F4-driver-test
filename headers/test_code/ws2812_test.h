/**
 * @file ws2812_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief WS2812 (neopixels) test code 
 * 
 * @version 0.1
 * @date 2023-03-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _WS2812_TEST_H_ 
#define _WS2812_TEST_H_ 

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define WS2812_SECOND_DEVICE 0    // Enable second device code 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief WS2812 setup code
 */
void ws2812_test_init(void); 


/**
 * @brief WS2812 test code 
 */
void ws2812_test_app(void); 

//=======================================================================================

#endif   // _WS2812_TEST_H_
