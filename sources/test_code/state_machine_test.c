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
// Functions 

void state_machine_test()
{
//     // Local variables 
//     static uint8_t arg_flag = CLEAR; 
//     static uint8_t arg_record = SET_BIT; 
//     static uint8_t num_args = CLEAR; 
//     static uint8_t arg_index = CLEAR; 
//     static uint8_t cmd_index = CLEAR; 
//     static uint16_t setter_status = CLEAR; 
//     static char user_input[HD44780U_USER_TEST_INPUT]; 
//     static char user_args[2][HD44780U_USER_TEST_INPUT]; 

//     static char line_input[4][HD44780U_USER_TEST_INPUT]; 
//     static hd44780u_cursor_offset_t line_offset[4]; 

//     // Check for a user command - try to put all of this in the state_machine_test 
//     if (uart_data_ready(USART2))
//     {
//         // Read the input 
//         uart_getstr(USART2, user_input, UART_STR_TERM_CARRIAGE);

//         // Looking for an argument and not a command 
//         if (arg_flag) 
//         {
//             // Assign user input to the argument buffer 
//             strcpy(user_args[arg_index], user_input); 

//             // Determine if there are more arguments 
//             if (++arg_index >= num_args) arg_flag = CLEAR; 
//         }

//         // Looking for a command and not an argument 
//         else 
//         {
//             // Look for a matching command 
//             for (cmd_index = 0; cmd_index < HD44780U_NUM_USER_CMDS; cmd_index++)
//             {
//                 if (str_compare(commands[cmd_index].cmd, user_input, BYTE_0)) break; 
//             }

//             // Check if a match was found 
//             if (cmd_index < HD44780U_NUM_USER_CMDS)
//             {
//                 // Execute command 
//                 if (cmd_index == (HD44780U_NUM_USER_CMDS-1))
//                 {
//                     for (uint8_t i = 0; i < (HD44780U_NUM_USER_CMDS-1); i++)
//                     {
//                         if ((setter_status >> i) & SET_BIT)
//                         {
//                             if (commands[i].setter != NULL) 
//                                 (commands[i].setter)(); 
//                             else 
//                                 (commands[i].data)(
//                                     line_input[i], 
//                                     line_offset[i]); 
//                         }
//                     }

//                     setter_status = CLEAR; 
//                 }

//                 // Another command 
//                 else 
//                 {
//                     // Set the setter flag to indicate the command chosen 
//                     setter_status |= (SET_BIT << cmd_index); 

//                     // Read the number of arguments 
//                     num_args = commands[cmd_index].arg_num; 

//                     if (num_args) 
//                     {
//                         arg_flag = SET_BIT; 
//                         arg_index = CLEAR; 
//                     }
//                 }
//             }
//         }

//         if (arg_flag) hd44780u_arg_prompt(); 
//         else hd44780u_cmd_prompt(); 
//     }

//     if (!arg_flag && !arg_record)
//     {
//         strcpy(line_input[cmd_index], user_args[0]); 
//         line_offset[cmd_index] = atoi(user_args[1]); 
//         arg_record = SET_BIT; 
//     }

//     if (arg_flag) arg_record = CLEAR; 
}

//================================================================================
