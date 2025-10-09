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

// Print user prompt 
void print_usr_prompt(void); 

// Print Bluetooth input 
void print_bt_input(void); 

// Reset at command parameters 
void clear_params(void); 

#if HC05_AT_ENABLE 

// Setup text 
void print_setup(void); 

// Parse the serial terminal input 
void parse_input(void); 

// Print the AT Command string 
void print_at_cmd_resp(void); 

#endif   // HC05_AT_ENABLE 

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


//=======================================================================================
// Setup code

void hc05_test_init()
{
    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    //==================================================
    
    //===================================================
    // Initialize timers 

    // General timer - 1us counter 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

#if HC05_AT_ENABLE 

    // Periodic (counter update) interrupt timer for user button status checks 
    tim_9_to_11_counter_init(
        TIM10, 
        TIM_84MHZ_100US_PSC, 
        0x0032,  // ARR=50, (50 counts)*(100us/count) = 5ms 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM10); 

    // Enable the interrupt handlers 
    nvic_config(TIM1_UP_TIM10_IRQn, EXTI_PRIORITY_1); 
    
    // Initialize interrupt handler flags 
    int_handler_init(); 

#endif   // HC05_AT_ENABLE 

    //==================================================

    //==================================================
    // Initialize UART 
    
    // UART2 for serial terminal communication 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR_BIT,             // STOP bits 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // UART1 for the HC05 module 
    uart_init(
        USART1, 
        GPIOA, 
        PIN_10, 
        PIN_9, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR_BIT,             // STOP bits 
        UART_FRAC_84_115200, 
        UART_MANT_84_115200, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    //===================================================

    //===================================================
    // HC05 initialization 
    
    // hc05 driver 
    hc05_init(
        USART1, 
        TIM9, 
        GPIOA,          // AT pin GPIO 
        PIN_8,          // AT pin 
        GPIOA,          // EN pin GPIO 
        PIN_12,         // EN pin 
        GPIOA,          // STATE pin GPIO 
        PIN_11);        // STATE pin 

    //===================================================

    //==================================================
    // GPIO initialization 

#if HC05_AT_ENABLE 

    // User button GPIO input 
    gpio_pin_init(GPIOC, PIN_0, MODER_INPUT, OTYPER_PP, OSPEEDR_HIGH, PUPDR_PU); 

    // Initialize the button debouncer 
    debounce_init(GPIOX_PIN_0); 

#endif   // HC05_AT_ENABLE 

    //==================================================

    //===================================================
    // Setup 

#if HC05_AT_ENABLE 

    print_setup(); 

#endif   // HC05_AT_ENABLE 
    
    clear_params(); 
    print_usr_prompt(); 

    //===================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void hc05_test_app()
{
    static uint8_t function = HC05_DATA_MODE; 

#if HC05_AT_ENABLE 

    //==================================================
    // User button input - mode selection 

    static uint8_t btn_block = CLEAR; 

    // Update user input button status 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 
        debounce((uint8_t)gpio_port_read(GPIOC)); 
    }

    // If button pressed after previously being released then change the mode 
    if (debounce_pressed((uint8_t)GPIOX_PIN_0) && !btn_block)
    {
        btn_block = SET_BIT; 

        function = 1 - function; 

        switch (function)
        {
            case HC05_DATA_MODE: 
                // Read the baud rate while in AT mode and use it here 

                hc05_change_mode(HC05_DATA_MODE, UART_BAUD_115200, UART_CLOCK_84); 
                uart_send_new_line(USART2); 
                uart_send_new_line(USART2); 
                uart_send_str(USART2, "Data mode\r\n\n"); 
                break; 
            
            case HC05_AT_CMD_MODE: 
                hc05_change_mode(HC05_AT_CMD_MODE, UART_BAUD_38400, UART_CLOCK_84); 
                uart_send_new_line(USART2); 
                uart_send_new_line(USART2); 
                uart_send_str(USART2, "AT mode\r\n\n"); 
                break; 
            
            default: 
                break; 
        }

        print_usr_prompt(); 
        hc05_clear(); 
    }

    // Free the button pressed status when the button is released after being pressed 
    if (debounce_released((uint8_t)GPIOX_PIN_0) && btn_block)
    {
        btn_block = CLEAR; 
    }

    //==================================================

#endif   // HC05_AT_ENABLE 

    //===================================================
    // Serial terminal data input 

    // Check if there is user input 
    if (uart_data_ready(USART2))
    {
        uart_get_data(USART2, buffer); 

        switch (function)
        {
            case HC05_DATA_MODE: 
                hc05_send(buffer); 
                uart_send_new_line(USART2); 
                break;

#if HC05_AT_ENABLE 
            case HC05_AT_CMD_MODE: 
                parse_input(); 
                hc05_at_command(command, operation, parameter, cmd_resp, HC05_AT_CMD_LEN); 
                uart_send_new_line(USART2); 
                print_at_cmd_resp(); 
                break; 
#endif   // HC05_AT_ENABLE 
            
            default:
                break;
        }

        print_usr_prompt(); 
        clear_params(); 
    }

    //===================================================

    //===================================================
    // Bluetooth terminal input 

    // Check if there is user input via Bluetooth and immediately read the data if there is 
    if (hc05_data_status())
    {
        hc05_read(bt_input, HC05_AT_CMD_LEN); 
        print_bt_input(); 
        print_usr_prompt(); 
        clear_params(); 
    }

    //===================================================
}

//=======================================================================================


//=======================================================================================
// Testing functions 

// Print user prompt 
void print_usr_prompt(void)
{
    uart_send_str(USART2, ">>> "); 
}


// Print the Bluetooth input 
void print_bt_input(void)
{
    uart_send_str(USART2, "Bluetooth input: "); 
    uart_send_str(USART2, bt_input); 
    uart_send_new_line(USART2); 
    uart_send_new_line(USART2); 
}


// Reset at command parameters 
void clear_params(void) 
{
    command = CLEAR; 
    operation = CLEAR; 
    memset((void *)buffer, CLEAR, HC05_AT_CMD_LEN); 
    memset((void *)parameter, CLEAR, HC05_AT_CMD_LEN); 
    memset((void *)cmd_resp, CLEAR, HC05_AT_CMD_LEN); 
    memset((void *)bt_input, CLEAR, HC05_AT_CMD_LEN); 
}


#if HC05_AT_ENABLE 

// Setup text 
void print_setup(void)
{
    uart_send_str(USART2, "AT Command Test\r\n\n"); 
    uart_send_str(USART2, "Legend: \r\n"); 
    uart_send_str(USART2, "- Format: <command> <operation> <parameter>\r\n"); 
    uart_send_str(USART2, "- Command: 0-34 --> See documentation\r\n"); 
    uart_send_str(USART2, "- Operation: 0-None, 1-Set, 2-Check\r\n"); 
    uart_send_str(USART2, "- Parameter: See documentation\r\n\n"); 
}


// Parse the user input 
void parse_input(void)
{
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
    uart_send_str(USART2, ">>> AT cmd response: "); 
    uart_send_str(USART2, cmd_resp); 
    uart_send_new_line(USART2); 
    uart_send_new_line(USART2); 
}

#endif   // HC05_AT_ENABLE 

//=======================================================================================
