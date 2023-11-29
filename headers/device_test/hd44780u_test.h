/**
 * @file hd44780u_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U LCD screen test code header 
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

// Conditional compilation 
#define HD44780U_CONTROLLER_TEST 1        // For switching between driver and controller testing 
#define HD44780U_BACKLIGHT_TEST 1         // Backlight control test 
#define HD44780U_DISPLAY_TEST 0           // Display on/off test 
#define HD44780U_CURSOR_TEST 0            // Cursor visibility test 
#define HD44780U_BLINK_TEST 0             // Cursor blink test 

// Controller 
#define HD44780U_NUM_USER_CMDS 19         // Number of defined user commands 
#define HD44780U_MAX_FUNC_PTR_ARGS 3      // Maximum arguments of all function pointer below 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Function pointer index 
 * 
 * @details This is used along with the state machine tester. It is specific to a device 
 *          test so the names change across test files. This enum is used to define the 
 *          function pointer index within state_request_t that corresponds to a user command. 
 *          An array instance of hd44780u_func_ptrs_t is made in the test code and this 
 *          index has to match the location of the function pointer in the array. 
 */
typedef enum {
    HD44780U_SET_PTR_1, 
    HD44780U_SET_PTR_2, 
    HD44780U_SET_PTR_3, 
    HD44780U_SET_PTR_4, 
    HD44780U_SET_PTR_5, 
    HD44780U_GET_PTR_1 
} hd44780u_func_ptr_index_t; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief HD44780U setter function pointer 1 
 * 
 * @details This function pointer is used for calling the following setters: 
 *          - hd44780u_line_set 
 */
typedef void (*hd44780u_set_ptr_1)(
    hd44780u_lines_t line, 
    char *line_data, 
    uint8_t offset); 


/**
 * @brief HD44780U setter function pointer 2 
 * 
 * @details This function pointer is used for calling the following setters: 
 *          - hd44780u_line_clear 
 */
typedef void (*hd44780u_set_ptr_2)(
    hd44780u_lines_t line); 


/**
 * @brief HD44780U setter function pointer 3 
 * 
 * @details This function pointer is used for calling the following setters: 
 *          - hd44780u_send_string 
 */
typedef void (*hd44780u_set_ptr_3)(
    char *print_string); 


/**
 * @brief HD44780U setter function pointer 4 
 * 
 * @details This function pointer is used for calling the following setters: 
 *          - hd44780u_cursor_pos 
 */
typedef void (*hd44780u_set_ptr_4)(
    hd44780u_line_start_position_t line_start, 
    uint8_t offset); 


/**
 * @brief HD44780U setter function pointer 5 
 * 
 * @details This function pointer is used for calling the following setters: 
 *           - hd44780u_clear 
 *           - hd44780u_display_on 
 *           - hd44780u_display_off 
 *           - hd44780u_cursor_on 
 *           - hd44780u_cursor_off 
 *           - hd44780u_blink_on 
 *           - hd44780u_blink_off 
 *           - hd44780u_backlight_on 
 *           - hd44780u_backlight_off 
 *           - hd44780u_set_write_flag 
 *           - hd44780u_set_low_pwr_flag 
 *           - hd44780u_clear_low_pwr_flag 
 *           - hd44780u_set_reset_flag 
 */
typedef void (*hd44780u_set_ptr_5)(void); 


/**
 * @brief HD44780U getter function pointer 1 
 * 
 * @details This function pointer is used for calling the following getters: 
 *           - hd44780u_get_state 
 */
typedef uint8_t (*hd44780u_get_ptr_1)(void); 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Structure of all HD44780U setter and getter function pointers (see above) 
 */
typedef struct hd44780u_func_ptrs_s 
{
    hd44780u_set_ptr_1 set1; 
    hd44780u_set_ptr_2 set2; 
    hd44780u_set_ptr_3 set3; 
    hd44780u_set_ptr_4 set4; 
    hd44780u_set_ptr_5 set5; 
    hd44780u_get_ptr_1 get1; 
}
hd44780u_func_ptrs_t; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HD44780U LCD sceen setup code 
 */
void hd44780u_test_init(void); 


/**
 * @brief HD44780U LCD sceen test code 
 */
void hd44780u_test_app(void); 

//=======================================================================================

#endif  // _HD44780U_TEST_H_ 
