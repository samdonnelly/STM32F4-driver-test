/**
 * @file hw125_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HW125 test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HW125_TEST_H_
#define _HW125_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

// Libraries 
#include "stdio.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Code options 
#define FORMAT_EXFAT 0

// File system 
#define BUFF_SIZE 1024 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HW125 setup code 
 * 
 */
void hw125_test_init(void); 


/**
 * @brief HW125 test code 
 * 
 */
void hw125_test_app(void); 

//=======================================================================================

#endif  // _HW125_TEST_H_ 
