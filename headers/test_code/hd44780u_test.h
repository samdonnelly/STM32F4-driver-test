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

#define HD44780U_CONTROLLER_TEST 0        // For switching between driver and controller testing 

#define HD44780U_NUM_USER_CMDS 15         // Number of defined user commands 
#define HD44780U_MAX_SETTER_ARGS 2        // Maximum arguments of all function pointer below 

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
    HD44780U_SETTER_PTR_1, 
    HD44780U_SETTER_PTR_2, 
    HD44780U_GETTER_PTR_1 
} hd44780u_func_ptr_index_t; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief HD44780U line clear and state flag function pointer 
 * 
 * @details This function pointer is used for calling the following setters from the device 
 *          controller: <br> 
 *           - hd44780u_line1_clear <br> 
 *           - hd44780u_line2_clear <br> 
 *           - hd44780u_line3_clear <br> 
 *           - hd44780u_line4_clear <br> 
 *           - hd44780u_set_write_flag <br> 
 *           - hd44780u_set_read_flag <br> 
 *           - hd44780u_set_reset_flag <br> 
 *           - hd44780u_set_low_pwr_flag <br> 
 *           - hd44780u_clear_low_pwr_flag 
 */
typedef void (*hd44780u_setter_ptr_1)(void); 


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
typedef void (*hd44780u_setter_ptr_2)(
    char *display_data, 
    hd44780u_cursor_offset_t line_offset); 


/**
 * @brief HD44780U getters function pointer 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: <br> 
 *           - hd44780u_get_state 
 */
typedef uint8_t (*hd44780u_getter_ptr_1)(void); 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Structure of all HD44780U setter and getter function pointers (see above) 
 */
typedef struct hd44780u_func_ptrs_s 
{
    hd44780u_setter_ptr_1 setter;          // Function pointer index 1 
    hd44780u_setter_ptr_2 data;            // Function pointer index 2 
    hd44780u_getter_ptr_1 getter;          // Function pointer index 3 
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
