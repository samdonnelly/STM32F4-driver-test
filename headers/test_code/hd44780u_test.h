/**
 * @file hd44780u_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief wayintop LCD test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HD44780U_TEST_H_
#define _HD44780U_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define LCD_NUM_LINES 4     // Number of lines on the LCD 

//=======================================================================================


//=======================================================================================
// Enums 
//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief wayintop LCD setup code 
 * 
 */
void hd44780u_test_init(void); 


/**
 * @brief wayintop LCD test code 
 * 
 */
void hd44780u_test_app(void); 

//=======================================================================================

#endif  // _HD44780U_TEST_H_ 
