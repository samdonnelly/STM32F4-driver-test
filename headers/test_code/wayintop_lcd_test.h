/**
 * @file wayintop_lcd_test.h
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

#ifndef _WAYINTOP_LCD_TEST_H_
#define _WAYINTOP_LCD_TEST_H_

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

/**
 * @brief LCD lines 
 */
typedef enum {
    LCD_L1,
    LCD_L2,
    LCD_L3,
    LCD_L4
} LCD_lines_t;

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief wayintop LCD setup code 
 * 
 */
void wayintop_lcd_test_init(void); 


/**
 * @brief wayintop LCD test code 
 * 
 */
void wayintop_lcd_test_app(void); 

//=======================================================================================

#endif  // _WAYINTOP_LCD_TEST_H_ 
