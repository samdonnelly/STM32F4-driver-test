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

#include "state_machine_test.h" 

#include <string.h>
#include <stdlib.h>

//=======================================================================================


//=======================================================================================
// Macros 

#define LCD_NUM_LINES 4               // Number of lines on the LCD 
#define HD44780U_TEST_CMD_LEN 15      // User command test input buffer size 
#define HD44780U_NUM_USER_CMDS 14     // Number of defined user commands 
#define HD44780U_USER_TEST_INPUT 25   // Max argument input size from user 

//=======================================================================================


//=======================================================================================
// Enums 
//=======================================================================================


//================================================================================
// Notes
// - Make the state_request_t structure general purpose by adding a bunch of generic 
//   function pointer names (such as function_pointer_1, function_pointer_2, etc.). 
// - Define the controller setter/getter function pointers in the test code file 
//   somewhere (like below). These will be specific in terms of arguments but generic 
//   in that they will match the names of the generic pointer names. 
// 
// 
//================================================================================


//================================================================================
// Function pointers 

/**
 * @brief 
 */
typedef void (*hd44780u_state_flag_tester)(void); 


/**
 * @brief 
 */
typedef void (*hd44780u_state_data_tester)(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 

//================================================================================


//================================================================================
// Structures 

/**
 * @brief 
 */
typedef struct hd44780u_state_request_s 
{
    char cmd[HD44780U_TEST_CMD_LEN];        // User command 
    uint8_t arg_num;                        // Number of arguments in function 
    hd44780u_state_flag_tester setter;      // Function pointer to flag setters 
    hd44780u_state_data_tester data;        // Function pointer to screen data setters 
}
hd44780u_state_request_t; 


/**
 * @brief 
 */
typedef struct hd44780u_func_ptrs_s 
{
    hd44780u_state_flag_tester setter;      // Function pointer to flag setters 
    hd44780u_state_data_tester data;        // Function pointer to screen data setters 
}
hd44780u_func_ptrs_t; 

//================================================================================


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
