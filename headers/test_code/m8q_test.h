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
#define M8Q_CONTROLLER_TEST 0     // Choose between driver and controller test code 
#define M8Q_MSG_COUNT 0           // Code that counts and prints number of incoming messages 
#define M8Q_DATA_CHECK 0          // Code that checks the data size available 
#define M8Q_TEST_LOCATION 1       // Code for testing positioning against waypoints 

// Controller testing 
#define M8Q_NUM_USER_CMDS 17      // Number of defined user commands 
#define M8Q_MAX_SETTER_ARGS 1     // Maximum arguments of all function pointer below 

// Location testing 
#define M8Q_TEST_NUM_WAYPOINTS 4  // Number of waypoints for location testing 

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
 * @brief M8Q getters function pointer 1 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_state
 */
typedef M8Q_STATE (*m8q_getter_ptr_1)(void); 


/**
 * @brief M8Q getters function pointer 2 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_fault_code 
 *           - m8q_get_navstat 
 */
typedef uint16_t (*m8q_getter_ptr_2)(void); 


/**
 * @brief M8Q getters function pointer 3 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_lat_str 
 *           - m8q_get_long_str 
 */
typedef void (*m8q_getter_ptr_3)(
    uint8_t *deg_min, 
    uint8_t *min_frac); 


/**
 * @brief M8Q getters function pointer 4 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_NS 
 *           - m8q_get_EW 
 */
typedef uint8_t (*m8q_getter_ptr_4)(void); 


/**
 * @brief M8Q getters function pointer 5 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_time 
 *           - m8q_get_date 
 */
typedef void (*m8q_getter_ptr_5)(
    uint8_t *utc); 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Structure of all M8Q setter and getter function pointers (see above) 
 */
typedef struct m8q_func_ptrs_s 
{
    m8q_setter_ptr_1 setter_1; 
    m8q_getter_ptr_1 getter_1; 
    m8q_getter_ptr_2 getter_2; 
    m8q_getter_ptr_3 getter_3; 
    m8q_getter_ptr_4 getter_4; 
    m8q_getter_ptr_5 getter_5; 
}
m8q_func_ptrs_t; 


/**
 * @brief Waypoint latitude and longitude 
 */
typedef struct m8q_test_waypoints_s 
{
    double lat; 
    double lon; 
}
m8q_test_waypoints_t; 

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
