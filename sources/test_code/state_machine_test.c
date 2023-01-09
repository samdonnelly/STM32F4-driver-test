/**
 * @file state_machine_test.c
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

//================================================================================
// Includes 

#include "state_machine_test.h"

//================================================================================


//================================================================================
// Function prototypes 

/**
 * @brief User command input prompt 
 */
void hd44780u_cmd_prompt(void); 


/**
 * @brief Setter argument input prompt 
 */
void hd44780u_arg_prompt(void); 

//================================================================================


//===============================================================================
// Globals 

/**
 * @brief State machine test control parameters 
 * 
 * @details This structure defines the information needed to control the operation of the 
 *          test code. It is used internally by the state machine tester. 
 */
typedef struct state_test_params_s 
{
    uint8_t display_flag;                      // Indicates when to display a user prompt 
    uint8_t display_mask;                      // Allows for getters to be printed before prompt 
    uint8_t num_usr_cmds;                      // Number of user defined commands for testing 
    uint8_t arg_flag;                          // Inidicates when command arguments are needed 
    uint8_t arg_record;                        // Triggers user input conversion 
    uint8_t num_args;                          // Number of arguments needed for a command 
    uint8_t arg_index;                         // Keeps track of arguments input by user 
    uint8_t cmd_index;                         // Indicates which command was invoked by the user 
    uint16_t set_get_status;                   // Keeps track of all requested user commands 
    char user_input[STATE_USER_TEST_INPUT];    // Buffer to store user input at terminal 
}
state_test_params_t; 


// State machine tester tracker 
static state_test_params_t test_params; 

//===============================================================================


//================================================================================
// Functions 

// State machine tester initialization 
void state_machine_init(
    uint8_t num_usr_cmds)
{
    // Pre-defined trackers 
    test_params.display_flag = CLEAR; 
    test_params.display_mask = CLEAR; 
    test_params.num_usr_cmds = num_usr_cmds; 
    test_params.arg_flag = CLEAR; 
    test_params.arg_record = SET_BIT; 
    test_params.num_args = CLEAR; 
    test_params.arg_index = CLEAR; 
    test_params.cmd_index = CLEAR; 
    test_params.set_get_status = CLEAR; 

    // Display the first user input prompt 
    hd44780u_cmd_prompt(); 
}


// State machine tester 
void state_machine_test(
    state_request_t state_request[], 
    char *user_args, 
    uint8_t *cmd_index, 
    uint8_t *arg_convert, 
    uint16_t *set_get_status)
{
    // Check for a user command 
    if (uart_data_ready(USART2))
    {
        // Read the input 
        uart_getstr(USART2, test_params.user_input, UART_STR_TERM_CARRIAGE);

        // Looking for an argument and not a command 
        if (test_params.arg_flag) 
        {
            // Assign user input to the argument buffer 
            user_args += (test_params.arg_index*STATE_USER_TEST_INPUT); 
            strcpy(user_args, test_params.user_input); 

            // Determine if there are more arguments 
            if (++test_params.arg_index >= test_params.num_args) 
                test_params.arg_flag = CLEAR; 

            test_params.display_flag = SET_BIT; 
        }

        // Looking for a command and not an argument 
        else 
        {
            // Look for a matching command 
            for (test_params.cmd_index = 0; 
                 test_params.cmd_index < test_params.num_usr_cmds; 
                 test_params.cmd_index++)
            {
                if (str_compare(
                        state_request[test_params.cmd_index].cmd, 
                        test_params.user_input, 
                        BYTE_0)) 
                    break; 
            }

            // Check if a match was found 
            if (test_params.cmd_index < test_params.num_usr_cmds)
            {
                // Execute command 
                if (test_params.cmd_index == (test_params.num_usr_cmds-1))
                {
                    *set_get_status = test_params.set_get_status; 
                    test_params.set_get_status = CLEAR; 
                    test_params.display_mask = SET_BIT; 
                }

                // Another command 
                else 
                {
                    // Set the setter/getter flag to indicate the command chosen 
                    test_params.set_get_status |= (SET_BIT << test_params.cmd_index); 

                    // Read the number of arguments 
                    test_params.num_args = state_request[test_params.cmd_index].arg_num; 

                    // If there are arguments then enter the argument input mode 
                    if (test_params.num_args) 
                    {
                        test_params.arg_flag = SET_BIT; 
                        test_params.arg_index = CLEAR; 
                    }

                    test_params.display_flag = SET_BIT; 
                }
            }

            else 
                test_params.display_flag = SET_BIT; 
        }
    }

    // Record the user command index 
    *cmd_index = test_params.cmd_index; 

    // Trigger an input convertion once all arguments have been entered 
    if (!test_params.arg_flag && !test_params.arg_record)
    {
        *arg_convert = SET_BIT; 
        test_params.arg_record = SET_BIT; 
    }
    else if (test_params.arg_flag) 
    {
        test_params.arg_record = CLEAR; 
    }

    // Display the user interface as needed 
    if (test_params.display_flag)
    {
        if (test_params.arg_flag) hd44780u_arg_prompt(); 
        else hd44780u_cmd_prompt(); 
        test_params.display_flag = CLEAR; 
    }
    else if (test_params.display_mask)
    {
        // If the display mask is set then the user prompt is delayed by one code cycle 
        test_params.display_flag = SET_BIT; 
        test_params.display_mask = CLEAR; 
    }
}


// Command input prompt 
void hd44780u_cmd_prompt(void)
{
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, "cmd >>> "); 
}


// Argument input prompt 
void hd44780u_arg_prompt(void)
{
    uart_send_new_line(USART2); 
    uart_sendstring(USART2, "arg >>> "); 
}

//================================================================================
