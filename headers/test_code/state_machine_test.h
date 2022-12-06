/**
 * @file state_machine_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief State machine test code 
 * 
 * @version 0.1
 * @date 2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _STATE_MACHINE_TEST_H_ 
#define _STATE_MACHINE_TEST_H_ 

//================================================================================
// Includes 

#include "includes_drivers.h"

//================================================================================


//================================================================================
// Macros 

#define STATE_TEST_CMD_LEN 15         // User command test input buffer size 
#define STATE_USER_TEST_INPUT 25      // 

//================================================================================


//================================================================================
// Enums 
//================================================================================


//===============================================================================
// Structures 

typedef struct state_test_params_s 
{
    uint8_t num_usr_cmds; 
    uint8_t arg_flag; 
    uint8_t arg_record; 
    uint8_t num_args; 
    uint8_t arg_index; 
    uint8_t cmd_index; 
    uint16_t setter_status; 
}
state_test_params_t; 


/**
 * @brief 
 */
typedef struct state_request_s 
{
    char cmd[STATE_TEST_CMD_LEN];        // Stores the possible user commands 
    uint8_t arg_num;                     // Number of arguments in function 
    uint8_t func_ptr_index;              // Which generic function pointer to use 
}
state_request_t; 

//===============================================================================


//================================================================================
// Functions 

/**
 * @brief 
 * 
 * @details 
 * 
 * @param num_usr_cmds 
 */
void state_machine_init(
    uint8_t num_usr_cmds); 


/**
 * @brief 
 * 
 * @details 
 * 
 * @param state_request : 
 */
void state_machine_test(
    state_request_t state_request[], 
    char *user_input, 
    char *user_args, 
    uint8_t *arg_convert, 
    uint16_t *setter_status); 

//================================================================================

#endif   // _STATE_MACHINE_TEST_H_
