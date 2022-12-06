/**
 * @file hd44780u_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief wayintop LCD test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "hd44780u_test.h"

//=======================================================================================


//===============================================================================
// Notes 
// - The screen is really sensitive to timing. Adding delays between write operations 
//   seems to be more reliable. 
//===============================================================================


//=======================================================================================
// Predefined text - can be defined here for use throughout the code 

static char* hd44780u_startup_screen[LCD_NUM_LINES] = 
{ 
    "Get",
    "ready", 
    "to", 
    "rumpus!" 
};


// static char* hd44780u_test_text[LCD_NUM_LINES] = 
// { 
//     "Rump",
//     "till", 
//     "you", 
//     "drop!" 
// };

//=======================================================================================


//================================================================================
// Globals 

// User command table 
static hd44780u_state_request_t commands[HD44780U_NUM_USER_CMDS] = 
{
    {"line1_set",   2, NULL,                         &hd44780u_line1_set, NULL}, 
    {"line2_set",   2, NULL,                         &hd44780u_line2_set}, 
    {"line3_set",   2, NULL,                         &hd44780u_line3_set}, 
    {"line4_set",   2, NULL,                         &hd44780u_line4_set}, 
    {"line1_clear", 0, &hd44780u_line1_clear,        NULL}, 
    {"line2_clear", 0, &hd44780u_line2_clear,        NULL}, 
    {"line3_clear", 0, &hd44780u_line3_clear,        NULL}, 
    {"line4_clear", 0, &hd44780u_line4_clear,        NULL}, 
    {"write",       0, &hd44780_set_write_flag,      NULL}, 
    {"read",        0, &hd44780u_set_read_flag,      NULL}, 
    {"reset",       0, &hd44780u_set_reset_flag,     NULL}, 
    {"lp_set",      0, &hd44780u_set_low_pwr_flag,   NULL}, 
    {"lp_clear",    0, &hd44780u_clear_low_pwr_flag, NULL}, 
    {"execute",     0, NULL,                         NULL} 
}; 


// User command table 
static state_request_t state_cmds[HD44780U_NUM_USER_CMDS] = 
{
    {"line1_set",   2, 2}, 
    {"line2_set",   2, 2}, 
    {"line3_set",   2, 2}, 
    {"line4_set",   2, 2}, 
    {"line1_clear", 0, 1}, 
    {"line2_clear", 0, 1}, 
    {"line3_clear", 0, 1}, 
    {"line4_clear", 0, 1}, 
    {"write",       0, 1}, 
    {"read",        0, 1}, 
    {"reset",       0, 1}, 
    {"lp_set",      0, 1}, 
    {"lp_clear",    0, 1}, 
    {"execute",     0, 0} 
}; 


// User command table 
static hd44780u_func_ptrs_t state_func[HD44780U_NUM_USER_CMDS] = 
{
    {NULL, &hd44780u_line1_set}, 
    {NULL, &hd44780u_line2_set}, 
    {NULL, &hd44780u_line3_set}, 
    {NULL, &hd44780u_line4_set}, 
    {&hd44780u_line1_clear, NULL}, 
    {&hd44780u_line2_clear, NULL}, 
    {&hd44780u_line3_clear, NULL}, 
    {&hd44780u_line4_clear, NULL}, 
    {&hd44780_set_write_flag, NULL}, 
    {&hd44780u_set_read_flag, NULL}, 
    {&hd44780u_set_reset_flag, NULL}, 
    {&hd44780u_set_low_pwr_flag, NULL}, 
    {&hd44780u_clear_low_pwr_flag, NULL}, 
    {NULL, NULL} 
}; 

//================================================================================


// Setup code
void hd44780u_test_init()
{
    // Setup code for the hd44780u_test here 

    //=================================================
    // Peripheral initialization 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);  // Serial terminal comms 

    // I2C1 init
    i2c1_init(
        I2C1, 
        I2C1_SDA_PB9,
        I2C1_SCL_PB8,
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

    // wayintop LCD screen init. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH);

    // Initialize the device controller 
    hd44780u_controller_init(); 

    // 
    state_machine_init(HD44780U_NUM_USER_CMDS); 

    //=================================================

    //=================================================
    // Screen/UI initialization - Move to the contoller when it's made 

    // Screen startup message - Use in controller init state 
    hd44780u_line_set(
        HD44780U_L1, 
        (char *)(hd44780u_startup_screen[HD44780U_L1]), 
        HD44780U_CURSOR_OFFSET_3); 
    hd44780u_line_set(
        HD44780U_L2, 
        (char *)(hd44780u_startup_screen[HD44780U_L2]), 
        HD44780U_CURSOR_OFFSET_0); 
    hd44780u_line_set(
        HD44780U_L3, 
        (char *)(hd44780u_startup_screen[HD44780U_L3]), 
        HD44780U_CURSOR_OFFSET_6); 
    hd44780u_line_set(
        HD44780U_L4, 
        (char *)(hd44780u_startup_screen[HD44780U_L4]), 
        HD44780U_CURSOR_OFFSET_7); 

    // Send all lines of data 
    hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L1); 

    hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L2); 
    
    hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L3); 
    
    hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_OFFSET_0);
    hd44780u_send_line(HD44780U_L4); 

    // Give time for the startup message to display then clear the message 
    tim_delay_ms(TIM9, 2000); 
    hd44780u_clear();
    tim_delay_ms(TIM9, 500);  // Adding this delay helps the screen transition to test_app 

    //=================================================
} 


// Test code 
void hd44780u_test_app()
{
    // Test code for the hd44780u_test here 

    //==================================================
    // Driver test code 

    // // Local variables 
    // static int8_t counter = 0;

    // // Print each line one at a time followed by a delay 
    // switch (counter)
    // {
    //     // Text is cast to a char pointer for use in the send_string function because 
    //     // its declaration in the header defaults to an int type. 

    //     case HD44780U_L1:
    //         hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_OFFSET_10);
    //         hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L1]));
    //         break;
        
    //     case HD44780U_L2:
    //         hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_OFFSET_8);
    //         hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L2])); 
    //         break;
        
    //     case HD44780U_L3:
    //         hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_OFFSET_6);
    //         hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L3])); 
    //         break;
        
    //     case HD44780U_L4:
    //         hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_OFFSET_4);
    //         hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L4])); 
    //         break;

    //     default:
    //         hd44780u_clear();
    //         counter = -1;
    //         break;
    // }

    // // Increment to next line
    // counter++;

    // // Delay for 1 second 
    // tim_delay_ms(TIM9, 1000);

    //==================================================


    //==================================================
    // Controller test code 

    // Local variables 
    static uint8_t arg_flag = CLEAR; 
    static uint8_t arg_record = SET_BIT; 
    static uint8_t num_args = CLEAR; 
    static uint8_t arg_index = CLEAR; 
    static uint8_t cmd_index = CLEAR; 
    static uint16_t setter_status = CLEAR; 
    static char user_input[HD44780U_USER_TEST_INPUT]; 
    static char user_args[2][HD44780U_USER_TEST_INPUT]; 

    static char line_input[4][HD44780U_USER_TEST_INPUT]; 
    static hd44780u_cursor_offset_t line_offset[4]; 

    // Check for a user command - try to put all of this in the state_machine_test 
    if (uart_data_ready(USART2))
    {
        // Read the input 
        uart_getstr(USART2, user_input, UART_STR_TERM_CARRIAGE);

        // Looking for an argument and not a command 
        if (arg_flag) 
        {
            // Assign user input to the argument buffer 
            strcpy(user_args[arg_index], user_input); 

            // Determine if there are more arguments 
            if (++arg_index >= num_args) arg_flag = CLEAR; 
        }

        // Looking for a command and not an argument 
        else 
        {
            // Look for a matching command 
            for (cmd_index = 0; cmd_index < HD44780U_NUM_USER_CMDS; cmd_index++)
            {
                if (str_compare(commands[cmd_index].cmd, user_input, BYTE_0)) break; 
            }

            // Check if a match was found 
            if (cmd_index < HD44780U_NUM_USER_CMDS)
            {
                // Execute command 
                if (cmd_index == (HD44780U_NUM_USER_CMDS-1))
                {
                    for (uint8_t i = 0; i < (HD44780U_NUM_USER_CMDS-1); i++)
                    {
                        if ((setter_status >> i) & SET_BIT)
                        {
                            // replace this with a return value and evaluate in the driver test 
                            if (commands[i].setter != NULL) 
                                (commands[i].setter)(); 
                            else 
                                (commands[i].data)(
                                    line_input[i], 
                                    line_offset[i]); 
                        }
                    }

                    setter_status = CLEAR; 
                }

                // Another command 
                else 
                {
                    // Set the setter flag to indicate the command chosen 
                    setter_status |= (SET_BIT << cmd_index); 

                    // Read the number of arguments 
                    num_args = commands[cmd_index].arg_num; 

                    if (num_args) 
                    {
                        arg_flag = SET_BIT; 
                        arg_index = CLEAR; 
                    }
                }
            }
        }

        if (arg_flag) hd44780u_arg_prompt(); 
        else hd44780u_cmd_prompt(); 
    }

    // Check if setter status has been set (non-zero) to know when to call setters 
    // Clear the setter status to zero when done 
    // Incorporate the state function array 

    // for (uint8_t i = 0; i < (test_params.num_usr_cmds-1); i++)
    // {
    //     if ((test_params.setter_status >> i) & SET_BIT)
    //     {
    //         if (state_request[i].setter != NULL) 
    //             (state_request[i].setter)(); 
    //         else 
    //             (state_request[i].data)(
    //                 line_input[i], 
    //                 line_offset[i]); 
    //     }
    // }

    // Check arg_convert to see if user argument input should be converted and assigned 

    if (!arg_flag && !arg_record)
    {
        strcpy(line_input[cmd_index], user_args[0]); 
        line_offset[cmd_index] = atoi(user_args[1]); 
        arg_record = SET_BIT; 
    }

    if (arg_flag) arg_record = CLEAR; 

    // Call the device controller 
    hd44780u_controller(); 

    //==================================================
}
