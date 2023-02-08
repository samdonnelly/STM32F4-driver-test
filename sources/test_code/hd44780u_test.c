/**
 * @file hd44780u_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief HD44780U LCD screen test code 
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


//=======================================================================================
// Globals 

static char* hd44780u_startup_screen[HD44780U_NUM_LINES] = 
{ 
    "Get",
    "ready", 
    "to", 
    "rumpus!" 
};

#if HD44780U_CONTROLLER_TEST

// User command table 
static state_request_t state_cmds[HD44780U_NUM_USER_CMDS] = 
{
    {"line1_set",   2, HD44780U_SETTER_PTR_2, 0}, 
    {"line2_set",   2, HD44780U_SETTER_PTR_2, 1}, 
    {"line3_set",   2, HD44780U_SETTER_PTR_2, 2}, 
    {"line4_set",   2, HD44780U_SETTER_PTR_2, 3}, 
    {"line1_clear", 0, HD44780U_SETTER_PTR_1, 0}, 
    {"line2_clear", 0, HD44780U_SETTER_PTR_1, 0}, 
    {"line3_clear", 0, HD44780U_SETTER_PTR_1, 0}, 
    {"line4_clear", 0, HD44780U_SETTER_PTR_1, 0}, 
    {"write",       0, HD44780U_SETTER_PTR_1, 0}, 
    {"read",        0, HD44780U_SETTER_PTR_1, 0}, 
    {"reset",       0, HD44780U_SETTER_PTR_1, 0}, 
    {"lp_set",      0, HD44780U_SETTER_PTR_1, 0}, 
    {"lp_clear",    0, HD44780U_SETTER_PTR_1, 0}, 
    {"state",       0, HD44780U_GETTER_PTR_1, 0}, 
    {"execute", 0, 0, 0} 
}; 


// User command table 
static hd44780u_func_ptrs_t state_func[HD44780U_NUM_USER_CMDS] = 
{
    {NULL, &hd44780u_line1_set, NULL}, 
    {NULL, &hd44780u_line2_set, NULL}, 
    {NULL, &hd44780u_line3_set, NULL}, 
    {NULL, &hd44780u_line4_set, NULL}, 
    {&hd44780u_line1_clear, NULL, NULL}, 
    {&hd44780u_line2_clear, NULL, NULL}, 
    {&hd44780u_line3_clear, NULL, NULL}, 
    {&hd44780u_line4_clear, NULL, NULL}, 
    {&hd44780u_set_write_flag, NULL, NULL}, 
    {&hd44780u_set_read_flag, NULL, NULL}, 
    {&hd44780u_set_reset_flag, NULL, NULL}, 
    {&hd44780u_set_low_pwr_flag, NULL, NULL}, 
    {&hd44780u_clear_low_pwr_flag, NULL, NULL}, 
    {NULL, NULL, &hd44780u_get_state}, 
    {NULL, NULL, NULL} 
}; 

#else 

static char* hd44780u_test_text[HD44780U_NUM_LINES] = 
{ 
    "Rump",
    "till", 
    "you", 
    "drop!" 
};

#endif 

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

    // LCD screen init. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH);

#if HD44780U_CONTROLLER_TEST

    // Initialize the device controller 
    hd44780u_controller_init(); 

    // Initialize the state machine test code 
    state_machine_init(HD44780U_NUM_USER_CMDS); 

#endif

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

#if HD44780U_CONTROLLER_TEST

    //==================================================
    // Controller test code 

    // Local variables 

    // General purpose arguments array 
    static char user_args[HD44780U_MAX_SETTER_ARGS][STATE_USER_TEST_INPUT]; 

    // Arguments for the hd44780u_state_data_tester function pointer 
    static char line_input[HD44780U_NUM_LINES][STATE_USER_TEST_INPUT]; 
    static hd44780u_cursor_offset_t line_offset[HD44780U_NUM_LINES]; 

    // Control flags 
    uint8_t arg_convert = 0; 
    uint32_t set_get_status = 0; 
    uint8_t cmd_index = 0; 
    uint8_t state = 0; 

    // Determine what to do from user input 
    state_machine_test(state_cmds, user_args[0], &cmd_index, &arg_convert, &set_get_status); 

    // Check if there are any setters or getters requested 
    if (set_get_status)
    {
        for (uint8_t i = 0; i < (HD44780U_NUM_USER_CMDS-1); i++)
        {
            if ((set_get_status >> i) & SET_BIT)
            {
                switch (state_cmds[i].func_ptr_index)
                {
                    case HD44780U_SETTER_PTR_1: 
                        (state_func[i].setter)(); 
                        break; 

                    case HD44780U_SETTER_PTR_2: 
                        (state_func[i].data)(
                            line_input[state_cmds[i].arg_buff_index], 
                            line_offset[state_cmds[i].arg_buff_index]); 
                        break; 

                    case HD44780U_GETTER_PTR_1: 
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
    hd44780u_controller(); 

    //==================================================

#else 

   //==================================================
    // Driver test code 

    // Local variables 
    static int8_t counter = 0;

    // Print each line one at a time followed by a delay 
    switch (counter)
    {
        // Text is cast to a char pointer for use in the send_string function because 
        // its declaration in the header defaults to an int type. 

        case HD44780U_L1:
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_OFFSET_10);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L1]));
            break;
        
        case HD44780U_L2:
            hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_OFFSET_8);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L2])); 
            break;
        
        case HD44780U_L3:
            hd44780u_cursor_pos(HD44780U_START_L3, HD44780U_CURSOR_OFFSET_6);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L3])); 
            break;
        
        case HD44780U_L4:
            hd44780u_cursor_pos(HD44780U_START_L4, HD44780U_CURSOR_OFFSET_4);
            hd44780u_send_string((char *)(hd44780u_test_text[HD44780U_L4])); 
            break;

        default:
            hd44780u_clear();
            counter = -1;
            break;
    }

    // Increment to next line
    counter++;

    // Delay for 1 second 
    tim_delay_ms(TIM9, 1000);

    //==================================================

#endif

}
