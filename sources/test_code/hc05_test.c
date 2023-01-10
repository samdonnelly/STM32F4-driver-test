/**
 * @file hc05_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HC05 test code 
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


static state_request_t state_cmds[HC05_NUM_USER_CMDS] =
{
    {"send",        2, HC05_FUNC_PTR_2, 0}, 
    {"read_set",    0, HC05_FUNC_PTR_1, 0}, 
    {"read_clear",  0, HC05_FUNC_PTR_1, 0}, 
    {"lp_set",      0, HC05_FUNC_PTR_1, 0}, 
    {"lp_clear",    0, HC05_FUNC_PTR_1, 0}, 
    {"reset",       0, HC05_FUNC_PTR_1, 0}, 
    {"state",       0, HC05_FUNC_PTR_3, 0}, 
    {"read_status", 0, HC05_FUNC_PTR_3, 0}, 
    {"read_data",   2, HC05_FUNC_PTR_2, 1}, 
    {"execute",     0, 0, 0} 
}; 

//=======================================================================================


//=======================================================================================
// Setup code

void hc05_test_init()
{
    //===================================================
    // Peripherals 
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 
    
    // UART2 for serial terminal communication 
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42); 

    // UART1 for the HC05 module 
    uart_init(USART1, UART_BAUD_115200, UART_CLOCK_84); 

    //===================================================

    //===================================================
    // HC05 initialization 
    
    // hc05 driver 
    hc05_init(USART1, GPIOA, PIN_8, GPIOA, PIN_12, GPIOA, PIN_11); 

#if HC05_CONTROLLER_TEST 

    // hc05 controller 
    hc05_controller_init(TIM9); 

    // State machine test 
    state_machine_init(HC05_NUM_USER_CMDS); 

#endif   // HC05_CONTROLLER_TEST

    //===================================================

    //===================================================
    // Setup 

#if HC05_CONTROLLER_TEST


#else   // HC05_CONTROLLER_TEST

    clear_params(); 
    print_data_input();

#endif   // HC05_CONTROLLER_TEST

    //===================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void hc05_test_app()
{
#if HC05_CONTROLLER_TEST 

    // Local variables 

    // General purpose arguments array 
    static char user_args[1][1]; 

    // Control flags 
    uint8_t arg_convert = 0; 
    uint16_t set_get_status = 0; 
    uint8_t cmd_index = 0; 
    uint8_t state = 0; 

    // Determine what to do from user input 
    state_machine_test(state_cmds, user_args[0], &cmd_index, &arg_convert, &set_get_status); 

    // Check if there are any setters or getters requested 
    if (set_get_status)
    {
        for (uint8_t i = 0; i < (HC05_NUM_USER_CMDS-1); i++)
        {
            if ((set_get_status >> i) & SET_BIT)
            {
                switch (state_cmds[i].func_ptr_index)
                {
                    case HC05_SETTER_PTR_1: 
                        (state_func[i].setter)(); 
                        break; 

                    case HC05_SETTER_PTR_2: 
                        (state_func[i].data)(
                            line_input[state_cmds[i].arg_buff_index], 
                            line_offset[state_cmds[i].arg_buff_index]); 
                        break; 

                    case HC05_GETTER_PTR_1: 
                        state = (state_func[i].getter)(); 
                        uart_sendstring(USART2, "\nState: "); 
                        uart_send_integer(USART2, (int16_t)state); 
                        uart_send_new_line(USART2); 

                    default: 
                        break; 
                }
            }
        }
    }

    // Check if user argument input should be converted and assigned 
    if (arg_convert)
    {
        switch (state_cmds[cmd_index].func_ptr_index)
        {
            case 1: 
                strcpy(line_input[state_cmds[cmd_index].arg_buff_index], user_args[0]); 
                line_offset[state_cmds[cmd_index].arg_buff_index] = atoi(user_args[1]); 
                break; 

            default: 
                break; 
        }
    }

    // Call the device controller 
    hc05_controller(); 

    // State check 

    // Check for the init state 
    // -> If in the init state then indicate init state 

    // Check for the read state 
    // -> If in the read state then check for the read status 
    // --> If the read status is true then read and print the data 

    // Check for the send state 
    // -> If in the send state then indicate send state 

    // Check for the reset state 
    // -> If in the reset state then indicate reset state 

#else   // HC05_CONTROLLER_TEST

    // Local variables 
    static uint8_t board_button = 0; 
    static uint8_t push = 0; 
    static uint8_t function = 0; 
    static uint8_t transition = 0; 

    //===================================================
    // Connection status - STATE pin 

    // Indicate the state pin status through the board LED 
    gpio_write(GPIOA, GPIOX_PIN_5, hc05_status()); 

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
                hc05_clear(); 
                break; 
            
            case 1:  // AT Command mode 
                hc05_change_mode(HC05_AT_CMD_MODE, UART_BAUD_38400, UART_CLOCK_84); 
                uart_send_new_line(USART2); 
                print_at_input(); 
                hc05_clear(); 
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
    if (uart_data_ready(USART2))
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
#if HC05_AT_EN 
                parse_input(); 
                hc05_at_command(command, operation, parameter, cmd_resp); 
                uart_send_new_line(USART2); 
                print_at_cmd_resp(); 
                print_at_input(); 
#endif   // HC05_AT_EN 
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
    if (hc05_data_status())
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

#endif   // HC05_CONTROLLER_TEST
}

//=======================================================================================


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

    // clear_buffer(parse_buffer); 
    memset(parse_buffer, NULL_CHAR, HC05_AT_CMD_LEN); 

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


// Reset at command parameters 
void clear_params(void) 
{
    command = CLEAR; 
    operation = CLEAR; 
    memset(buffer, NULL_CHAR, HC05_AT_CMD_LEN); 
    memset(parameter, NULL_CHAR, HC05_AT_CMD_LEN); 
    memset(cmd_resp, NULL_CHAR, HC05_AT_CMD_LEN); 
    memset(bt_input, NULL_CHAR, HC05_AT_CMD_LEN); 
}
