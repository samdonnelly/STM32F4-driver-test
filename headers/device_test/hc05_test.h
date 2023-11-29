/**
 * @file hc05_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC05 test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _HC05_TEST_H_
#define _HC05_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include "int_handlers.h"

#include "state_machine_test.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define HC05_CONTROLLER_TEST 0     // For switching between driver and controller testing 

#define HC05_NUM_USER_CMDS 10      // Number of defined user commands for controller test 
#define HC05_MAX_SETTER_ARGS 1     // Maximum arguments of all function pointer below 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief HC05 setter/getter function pointer 1 
 * 
 * @details This function pointer is used for calling the following setters from the device 
 *          controller: 
 *           - hc05_set_read
 *           - hc05_clear_read
 *           - hc05_set_low_power
 *           - hc05_clear_low_power
 *           - hc05_set_reset
 */
typedef void (*hc05_func_ptr_1)(void); 


/**
 * @brief HC05 setter/getter function pointer 2 
 * 
 * @details This function pointer is used for calling the following setters/getters from 
 *          the device controller: 
 *           - hc05_set_send
 *           - hc05_get_read_data
 */
typedef void (*hc05_func_ptr_2)(
    uint8_t *data_buffer, 
    uint8_t size); 


/**
 * @brief HC05 setter/getter function pointer 3 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - hc05_get_state
 *           - hc05_get_read_status
 */
typedef uint8_t (*hc05_func_ptr_3)(void); 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Structure of all HC05 setter/getter function pointers (see above) 
 */
typedef struct hc05_func_ptrs_s 
{
    hc05_func_ptr_1 func1;          // Function pointer index 1 
    hc05_func_ptr_2 func2;          // Function pointer index 2 
    hc05_func_ptr_3 func3;          // Function pointer index 3 
}
hc05_func_ptrs_t; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief HC05 setup code 
 */
void hc05_test_init(void); 


/**
 * @brief HC05 test code 
 */
void hc05_test_app(void); 

//=======================================================================================

#endif  // _HC05_TEST_H_ 
