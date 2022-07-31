/**
 * @file user_app.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once per loop
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "user_app.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

// Setup text 
void print_setup(void); 

// Print the user input prompt 
void print_input(void); 

// Parse the serial terminal input 
void parse_input(void); 

// Print the AT Command string 
void print_at_cmd(void); 

// Clear the buffer 
void clear_buffer(char *buff); 

//=======================================================================================


//=======================================================================================
// Global Variables 

char buffer[HC05_AT_CMD_LEN];     // String to hold user input 
uint8_t command = 0;              // AT Command to send 
uint8_t operation = 0;            // Operation of AT command 
char parameter[HC05_AT_CMD_LEN];  // Parameter of AT command
char at_cmd[HC05_AT_CMD_LEN];     // AT command string 

//=======================================================================================


// User application 
void user_app()
{
    // Local variables 
    static uint8_t run_once = 1; 

    // Run once 
    if (run_once)
    {
        clear_buffer(buffer); 
        print_setup(); 
        print_input(); 
        run_once = 0; 
    }

#if HC05_AT_CMD_MODE  // AT command code 
    // Check if there is data 
    if (USART2->SR & (SET_BIT << SHIFT_5))
    {
        // Read the string input 
        uart2_getstr(buffer); 
        parse_input(); 
        // hc05_at_command(command, operation, parameter, at_cmd); 
        uart2_send_new_line();
        // print_at_cmd(); 
        clear_buffer(buffer); 
        print_input(); 
    }
#else  // Data mode 
    uart2_send_new_line(); 
#endif  // HC05_AT_CMD_MODE
}


// Setup text 
void print_setup(void)
{
    uart2_sendstring("AT Command Test\r\n\n"); 
    uart2_sendstring("Legend: \r\n"); 
    uart2_sendstring("- Format: <command> <operation> <parameter>\r\n"); 
    uart2_sendstring("- Command: 0-34 --> See documentation\r\n"); 
    uart2_sendstring("- Operation: 0-None, 1-Set, 2-Check\r\n"); 
    uart2_sendstring("- Parameter: See documentation\r\n\n"); 
}


// Print the user input prompt 
void print_input(void)
{
    uart2_sendstring("AT Command Args: "); 
}


// Parse the user input 
void parse_input(void)
{
    // local variables 
    uint8_t parse_state = 0; 
    int8_t parse_index = 0; 
    char parse_buffer[15]; 
    uint16_t temp = 0; 

    clear_buffer(parse_buffer); 

    // TODO figure out why this is hard faulting when exiting the loop 
    for (uint8_t i = 0; buffer[i] != '\0'; i++)
    {
        switch (parse_state)
        {
        case 0:  // Command 
            // 
            if (buffer[i] == ' ')
            {
                parse_index--; 

                for (uint8_t j = 0; parse_index >= 0; j++)
                {
                    temp = ((uint8_t)(parse_buffer[parse_index]) - 48);

                    for (uint8_t k = 0; k < j; k++) temp *= 10;

                    // command += ((uint8_t)(parse_buffer[parse_index]) - 48);
                    command += temp; 
                    parse_index--; 
                }
                parse_state++; 
                parse_index = 0; 
            }
            else
            {
                parse_buffer[parse_index] = buffer[i]; 
                parse_index++; 
            }
            break;
        
        case 1:  // Operation 
            // 
            if (buffer[i] == ' ')
            {
                parse_index--; 

                for (uint8_t j = 0; parse_index >= 0; j++)
                {
                    temp = ((uint8_t)(parse_buffer[parse_index]) - 48);

                    for (uint8_t k = 0; k < j; k++) temp *= 10;
                    
                    // operation += ((uint8_t)(parse_buffer[parse_index]) - 48);
                    operation += temp; 
                    parse_index--; 
                }
                parse_state++; 
                parse_index = 0; 
            }
            else
            {
                parse_buffer[parse_index] = buffer[i]; 
                parse_index++; 
            }
            break;
        
        case 2:  // Parameter 
            parameter[parse_index] = buffer[i]; 
            parse_index++;
            break; 
        
        default:  // Unknown 
            break;
        }
    }
}


// Print the AT command string 
void print_at_cmd(void)
{
    uart2_sendstring("AT Command String: "); 
    uart2_send_new_line(); 
    uart2_send_new_line(); 
}


// Clear the string buffer 
void clear_buffer(char *buff)
{
    for (uint8_t i = 0; i < HC05_AT_CMD_LEN; i++)
    {
        *buff = '\0'; 
        buff++; 
    }
}
