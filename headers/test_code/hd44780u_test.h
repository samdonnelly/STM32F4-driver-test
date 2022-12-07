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

// Test code 
#include "state_machine_test.h" 

// Standard libraries 
#include <string.h>
#include <stdlib.h>

//=======================================================================================


//=======================================================================================
// Macros 

#define CONTROLLER_TEST 1             // For switching between driver and controller testing 

#define HD44780U_NUM_USER_CMDS 14     // Number of defined user commands 
#define HD44780U_MAX_SETTER_ARGS 2    // Maximum arguments of all function pointer below 

//=======================================================================================


//================================================================================
// Function pointers 

// TODO add a function pointer for a getter - result will be printing the getter to the terminal 

/**
 * @brief HD44780U line clear and state flag function pointer 
 * 
 * @details This function pointer is used for calling the following setters from the device 
 *          controller: <br> 
 *           - hd44780u_line1_clear <br> 
 *           - hd44780u_line2_clear <br> 
 *           - hd44780u_line3_clear <br> 
 *           - hd44780u_line4_clear <br> 
 *           - hd44780_set_write_flag <br> 
 *           - hd44780u_set_read_flag <br> 
 *           - hd44780u_set_reset_flag <br> 
 *           - hd44780u_set_low_pwr_flag <br> 
 *           - hd44780u_clear_low_pwr_flag 
 */
typedef void (*hd44780u_state_flag_tester)(void); 


/**
 * @brief HD44780U line content setter function pointer 
 * 
 * @details This function pointer is used for calling the following setters from the device 
 *          controller: <br> 
 *           - hd44780u_line1_set <br> 
 *           - hd44780u_line2_set <br> 
 *           - hd44780u_line3_set <br> 
 *           - hd44780u_line4_set 
 */
typedef void (*hd44780u_state_data_tester)(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 

//================================================================================


//================================================================================
// Structures 

/**
 * @brief Structure of all HD44780U setter function pointers (see above) 
 */
typedef struct hd44780u_func_ptrs_s 
{
    hd44780u_state_flag_tester setter; 
    hd44780u_state_data_tester data; 
}
hd44780u_func_ptrs_t; 

//================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HD44780U LCD sceen setup code 
 * 
 */
void hd44780u_test_init(void); 


/**
 * @brief HD44780U LCD sceen test code 
 * 
 */
void hd44780u_test_app(void); 

//=======================================================================================

#endif  // _HD44780U_TEST_H_ 
