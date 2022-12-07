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

// State machine tester tracker 
static state_test_params_t test_params; 

//===============================================================================


//================================================================================
// Functions 

// State machine tester initialization 
void state_machine_init(
    uint8_t num_usr_cmds)
{
    test_params.num_usr_cmds = num_usr_cmds; 
    test_params.arg_flag = CLEAR; 
    test_params.arg_record = SET_BIT; 
    test_params.num_args = CLEAR; 
    test_params.arg_index = CLEAR; 
    test_params.cmd_index = CLEAR; 
    test_params.setter_status = CLEAR; 

    hd44780u_cmd_prompt(); 
}


// State machine tester 
void state_machine_test(
    state_request_t state_request[], 
    char *user_args, 
    uint8_t *cmd_index, 
    uint8_t *arg_convert, 
    uint16_t *setter_status)
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
                    *setter_status = test_params.setter_status; 
                    test_params.setter_status = CLEAR; 
                }

                // Another command 
                else 
                {
                    // Set the setter flag to indicate the command chosen 
                    test_params.setter_status |= (SET_BIT << test_params.cmd_index); 

                    // Read the number of arguments 
                    test_params.num_args = state_request[test_params.cmd_index].arg_num; 

                    // If there are arguments then enter the argument input mode 
                    if (test_params.num_args) 
                    {
                        test_params.arg_flag = SET_BIT; 
                        test_params.arg_index = CLEAR; 
                    }
                }
            }
        }

        // Display the user input prompt 
        if (test_params.arg_flag) hd44780u_arg_prompt(); 
        else hd44780u_cmd_prompt(); 
    }

    // Trigger an input convertion once all arguments have been entered 
    if (!test_params.arg_flag && !test_params.arg_record)
    {
        *arg_convert = SET_BIT; 
        test_params.arg_record = SET_BIT; 
    }

    if (test_params.arg_flag) test_params.arg_record = CLEAR; 

    *cmd_index = test_params.cmd_index; 
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
