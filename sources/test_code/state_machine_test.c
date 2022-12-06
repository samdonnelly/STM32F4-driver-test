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
// Notes: 
// - How to use: 
//   - 
//================================================================================


//================================================================================
// Function prototypes 

void hd44780u_cmd_prompt(void); 

void hd44780u_arg_prompt(void); 

//================================================================================


//===============================================================================
// Globals 

static state_test_params_t test_params; 

//===============================================================================


//================================================================================
// Functions 

// 
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


void state_machine_test(
    state_request_t state_request[], 
    char *user_input, 
    uint8_t *arg_convert)
{
    // Local variables 
    // static char user_input[HD44780U_USER_TEST_INPUT]; 
    // static char user_args[2][HD44780U_USER_TEST_INPUT]; 

    // static char line_input[4][HD44780U_USER_TEST_INPUT]; 
    // static hd44780u_cursor_offset_t line_offset[4]; 

    // Check for a user command - try to put all of this in the state_machine_test 
    if (uart_data_ready(USART2))
    {
        // Read the input 
        uart_getstr(USART2, user_input, UART_STR_TERM_CARRIAGE);

        // Looking for an argument and not a command 
        if (test_params.arg_flag) 
        {
            // Assign user input to the argument buffer 
            // strcpy(user_args[test_params.arg_index], user_input); 

            // Determine if there are more arguments 
            if (++test_params.arg_index >= test_params.num_args) test_params.arg_flag = CLEAR; 
        }

        // Looking for a command and not an argument 
        else 
        {
            // Look for a matching command 
            for (test_params.cmd_index = 0; 
                 test_params.cmd_index < test_params.num_usr_cmds; 
                 test_params.cmd_index++)
            {
                if (str_compare(state_request[test_params.cmd_index].cmd, user_input, BYTE_0)) 
                    break; 
            }

            // Check if a match was found 
            if (test_params.cmd_index < test_params.num_usr_cmds)
            {
                // Execute command 
                if (test_params.cmd_index == (test_params.num_usr_cmds-1))
                {
                    for (uint8_t i = 0; i < (test_params.num_usr_cmds-1); i++)
                    {
                        if ((test_params.setter_status >> i) & SET_BIT)
                        {
                            // if (state_request[i].setter != NULL) 
                            //     (state_request[i].setter)(); 
                            // else 
                            //     (state_request[i].data)(
                            //         line_input[i], 
                            //         line_offset[i]); 
                        }
                    }

                    test_params.setter_status = CLEAR; 
                }

                // Another command 
                else 
                {
                    // Set the setter flag to indicate the command chosen 
                    test_params.setter_status |= (SET_BIT << test_params.cmd_index); 

                    // Read the number of arguments 
                    test_params.num_args = state_request[test_params.cmd_index].arg_num; 

                    if (test_params.num_args) 
                    {
                        test_params.arg_flag = SET_BIT; 
                        test_params.arg_index = CLEAR; 
                    }
                }
            }
        }

        if (test_params.arg_flag) hd44780u_arg_prompt(); 
        else hd44780u_cmd_prompt(); 
    }

    if (!test_params.arg_flag && !test_params.arg_record)
    {
        *arg_convert = SET_BIT; 
        test_params.arg_record = SET_BIT; 
    }

    if (test_params.arg_flag) test_params.arg_record = CLEAR; 
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
