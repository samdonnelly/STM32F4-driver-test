/**
 * @file m8q_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _M8Q_TEST_H_
#define _M8Q_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include "int_handlers.h"

// Test code 
#include "state_machine_test.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define M8Q_CONTROLLER_TEST 1     // Choose between driver and controller test code 
#define M8Q_MSG_COUNT 0           // Code that counts and prints number of incoming messages 
#define M8Q_DATA_CHECK 0          // Code that checks the data size available 
#define M8Q_TEST_OTHER 0          // Other test code 

#define M8Q_NUM_USER_CMDS 10      // Number of defined user commands 
#define M8Q_MAX_SETTER_ARGS 1     // Maximum arguments of all function pointer below 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief Function pointer index 
 * 
 * @details This is used along with the state machine tester. It is specific to a device 
 *          test so the names change across test files. This enum is used to define the 
 *          function pointer index within state_request_t that corresponds to a user command. 
 *          An array instance of m8q_func_ptrs_t is made in the test code and this 
 *          index has to match the location of the function pointer in the array. 
 */
typedef enum {
    M8Q_SETTER_PTR_1, 
    M8Q_GETTER_PTR_1 
} m8q_func_ptr_index_t; 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief M8Q line clear and state flag function pointer 
 * 
 * @details This function pointer is used for calling the following setters from the device 
 *          controller: 
 *           - m8q_set_read_ready 
 *           - m8q_clear_read_ready 
 *           - m8q_set_read_flag 
 *           - m8q_clear_read_flag 
 *           - m8q_set_low_pwr_flag 
 *           - m8q_clear_low_pwr_flag 
 *           - m8q_set_reset_flag 
 */
typedef void (*m8q_setter_ptr_1)(void); 


/**
 * @brief M8Q getters function pointer 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_state
 *           - m8q_get_fault_code 
 */
typedef uint16_t (*m8q_getter_ptr_1)(void); 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Structure of all M8Q setter and getter function pointers (see above) 
 */
typedef struct m8q_func_ptrs_s 
{
    m8q_setter_ptr_1 setter;          // Function pointer index 1 
    m8q_getter_ptr_1 getter;          // Function pointer index 2 
}
m8q_func_ptrs_t; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief M8Q setup code 
 */
void m8q_test_init(void); 


/**
 * @brief M8Q test code 
 */
void m8q_test_app(void); 

//=======================================================================================

#endif  // _M8Q_TEST_H_ 
