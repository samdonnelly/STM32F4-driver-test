/**
 * @file hc05_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC-05 test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "hc05_test.h"

//=======================================================================================


//===================================================
// Notes 
// - State machine: 
//   - Have a state that is called at the end of AT command mode state where the baud 
//     rate is read and used to set the baud rate for data mode 
//===================================================


//=======================================================================================
// Function prototypes 

// Setup text 
void print_setup(void); 

// Print the user input prompt for Data mode 
void print_data_input(void); 

// Print the user input prompt for AT Command mode 
void print_at_input(void); 

// Parse the serial terminal input 
void parse_input(void); 

// Print the AT Command string 
void print_at_cmd_resp(void); 

// Print Bluetooth input 
void print_bt_input(void); 

// Clear the buffer 
void clear_buffer(char *buff); 

// Reset at command parameters 
void clear_params(void); 

//=======================================================================================


//=======================================================================================
// Global Variables 

char buffer[HC05_AT_CMD_LEN];       // String to hold user input 
uint8_t command;                    // AT Command to send 
uint8_t operation;                  // Operation of AT command 
char parameter[HC05_AT_CMD_LEN];    // Parameter of AT command
char cmd_resp[HC05_AT_CMD_LEN];     // AT command string 
char bt_input[HC05_AT_CMD_LEN];     // Bluetooth input 

//=======================================================================================


// Setup code
void hc05_test_init()
{
    // Setup code for the hc05_test here 

    //===================================================
    // Other drivers 
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    //===================================================

    //===================================================
    // Communication drivers 
    
    // Serial terminal communication 
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42); 

    // HC-05 module 
    uart_init(USART1, UART_BAUD_115200, UART_CLOCK_84);  

    //===================================================

    //===================================================
    // Device drivers 
    
    // HC-05 module 
    hc05_init(USART1, HC05_PIN34_ENABLE, HC05_EN_ENABLE, HC05_STATE_ENABLE); 

    //===================================================
} 


// Test code 
void hc05_test_app()
{
    // Test code for the hc05_test here 

    // Local variables 
    static uint8_t run_once = 1; 
    static uint8_t state_pin = 0; 
    static uint8_t board_button = 0; 
    static uint8_t push = 0; 
    static uint8_t function = 0; 
    static uint8_t transition = 0; 

    // Run once 
    if (run_once)
    {
        clear_params(); 
        print_data_input();
        uart_clear_dr(USART1);  // needed at the beginning 
        run_once = 0; 
    }

    //===================================================
    // Connection status - STATE pin 
    
    // Read the STATE pin 
    state_pin = gpio_read(GPIOA, GPIOX_PIN_11); 

    // Indicate the status of the STATE pin using the board green LED 
    if (state_pin) gpio_write(GPIOA, GPIOX_PIN_5, GPIO_HIGH);  // Module connected 
    else gpio_write(GPIOA, GPIOX_PIN_5, GPIO_LOW);  // Module disconnected 

    //===================================================

    //===================================================
    // Mode selection - push button 
    
    // Read the board push button 
    board_button = gpio_read(GPIOC, GPIOX_PIN_13); 

    // Button pushed 
    if (!board_button && !push)
    {
        function = 1 - function; 
        push++; 
        transition++; 
    }
    // Button released 
    else if (board_button && push)
    {
        push = 0; 
    }

    if (transition) 
    {
        switch (function)
        {
            case 0:  // Data mode 
                // TODO read the baud rate while in AT mode and use it here 
                hc05_change_mode(HC05_DATA_MODE, UART_BAUD_115200, UART_CLOCK_84); 
                uart_send_new_line(USART2); 
                print_data_input();  
                uart_clear_dr(USART1);
                break; 
            
            case 1:  // AT Command mode 
                hc05_change_mode(HC05_AT_CMD_MODE, UART_BAUD_38400, UART_CLOCK_84); 
                uart_send_new_line(USART2); 
                print_at_input(); 
                uart_clear_dr(USART1);
                break; 
            
            default: 
                break; 
        }
        transition = 0; 
    }

    //===================================================

    //===================================================
    // Serial terminal data input 

    // Check if there is user input 
    if (USART2->SR & (SET_BIT << SHIFT_5))
    {
        uart_getstr(USART2, buffer, UART_STR_TERM_CARRIAGE); 

        switch (function)
        {
            case 0:  // Data mode 
                hc05_send(buffer); 
                uart_send_new_line(USART2); 
                print_data_input(); 
                break;

            case 1:  // AT Command mode 
#if HC05_AT_EN  // AT command code 
                parse_input(); 
                hc05_at_command(command, operation, parameter, cmd_resp); 
                uart_send_new_line(USART2); 
                print_at_cmd_resp(); 
                print_at_input(); 
#endif  // HC05_AT_CMD_MODE
                break; 
            
            default:
                break;
        }

        clear_params(); 
    }

    //===================================================

    //===================================================
    // Bluetooth terminal input 

    // Check if there is user input via Bluetooth 
    if (USART1->SR & (SET_BIT << SHIFT_5))
    {
        hc05_read(bt_input);  // Must immediately read the data so it's not lost 
        uart_send_new_line(USART2); 
        uart_send_new_line(USART2); 
        print_bt_input(); 
        uart_send_new_line(USART2); 
        uart_send_new_line(USART2); 
        print_data_input(); 
        clear_params(); 
    }

    //===================================================
}


// Setup text 
void print_setup(void)
{
    uart_sendstring(USART2, "AT Command Test\r\n\n"); 
    uart_sendstring(USART2, "Legend: \r\n"); 
    uart_sendstring(USART2, "- Format: <command> <operation> <parameter>\r\n"); 
    uart_sendstring(USART2, "- Command: 0-34 --> See documentation\r\n"); 
    uart_sendstring(USART2, "- Operation: 0-None, 1-Set, 2-Check\r\n"); 
    uart_sendstring(USART2, "- Parameter: See documentation\r\n\n"); 
}


// Print the user input prompt for Data mode 
void print_data_input(void)
{
    uart_sendstring(USART2, "Data to send: "); 
}


// Print the user input prompt for AT Command mode 
void print_at_input(void)
{
    uart_sendstring(USART2, "AT Command Args: "); 
}


// Parse the user input 
void parse_input(void)
{
    // local variables 
    uint8_t parse_state = 0; 
    int8_t parse_index = 0; 
    char parse_buffer[HC05_AT_CMD_LEN]; 
    uint16_t temp = 0; 

    clear_buffer(parse_buffer); 

    for (uint8_t i = 0; buffer[i] != '\0'; i++)
    {
        switch (parse_state)
        {
        case 0:  // Command 
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
            if ((buffer[i] != '\r') || (buffer[i] != ' '))
            {
                parameter[parse_index] = buffer[i]; 
                parse_index++;
            }
            break; 
        
        default:  // Unknown 
            break;
        }
    }
}


// Print the AT command response 
void print_at_cmd_resp(void)
{
    uart_sendstring(USART2, "AT Command Response: "); 
    uart_sendstring(USART2, cmd_resp); 
    uart_send_new_line(USART2); 
}


// Print the Bluetooth input 
void print_bt_input(void)
{
    uart_sendstring(USART2, "Bluetooth input: "); 
    uart_sendstring(USART2, bt_input); 
    uart_send_new_line(USART2); 
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


// Reset at command parameters 
void clear_params(void) 
{
    clear_buffer(buffer); 
    command = 0; 
    operation = 0; 
    clear_buffer(parameter); 
    clear_buffer(cmd_resp); 
    clear_buffer(bt_input); 
}
