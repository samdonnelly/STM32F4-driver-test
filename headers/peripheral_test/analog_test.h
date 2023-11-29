/**
 * @file analog_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Analog test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _ANALOG_TEST_H_
#define _ANALOG_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define ANALOG_TEST_MODE_1 0         // Single read, scan disabled 
#define ANALOG_TEST_MODE_2 0         // Single read, scan enabled 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Analog test setup code 
 */
void analog_test_init(void); 


/**
 * @brief Analog test code 
 */
void analog_test_app(void); 

//=======================================================================================

#endif  // _ANALOG_TEST_H_ 
