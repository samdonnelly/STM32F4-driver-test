/**
 * @file nrf24l01_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 RF module test code 
 * 
 * @details There are multiple tests and each test has code for two separate radio 
 *          transceivers that are meant to wirelessly communicate with one another. These 
 *          transceivers are meant to be controlled by two separate systems/controllers. 
 *          Device one code should be flashed to system one and device two code to system 
 *          two. 
 * 
 * @version 0.1
 * @date 2023-08-23
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "nrf24l01_test.h" 
#include "nrf24l01_config.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

//==================================================
// Conditional compilation 

// Device 
#define NRF24L01_SYSTEM_1 1           // Enable device 1 code 
#define NRF24L01_SYSTEM_2 0           // Enable device 2 code 

// Test code 
#define NRF24L01_HEARTBEAT 0          // Heartbeat 
#define NRF24L01_MANUAL_CONTROL 1     // SD card on same SPI bus but different pins 

// Hardware 
#define NRF24L01_TEST_SCREEN 0        // HD44780U screen in the system - shuts screen off 

//==================================================

// Configuration 
#define NRF24L01_RF_FREQ 10           // Comm frequency: 2400 MHz + this value (MHz) 

// User commands 
#define NUM_USER_CMDS 4               // Number of test states 

//=======================================================================================


//=======================================================================================
// Global Variables 

// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Timing information 
    TIM_TypeDef *timer_nonblocking;                // Timer used for non-blocking delays 
    tim_compare_t delay_timer;                     // Delay timing info 

    // Configuration 
    nrf24l01_data_pipe_t pipe; 

    // Payload data 
    uint8_t read_buff[NRF24L01_MAX_PAYLOAD_LEN];   // Data read by PRX from PTX device 
}
nrf24l01_test_trackers_t; 

// Device tracker instance 
static nrf24l01_test_trackers_t nrf24l01_test; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

// Setup and looped code for each test 
void nrf24l01_heartbeat_test_init(void); 
void nrf24l01_manual_control_test_init(void); 
void nrf24l01_heartbeat_test_loop(void); 
void nrf24l01_manual_control_test_loop(void); 


/**
 * @brief User terminal prompt 
 */
void nrf24l01_test_user_prompt(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

void nrf24l01_test_init(void)
{
    // Universal (to all nrf24l01 tests) setup code 

    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // General purpose timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Configure the board LED to blink when transmitting. 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
    gpio_write(GPIOA, GPIOX_PIN_5, GPIO_LOW); 
    
    //==================================================

    //==================================================
    // UART initialization 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE);   // DMA enabled so it can be configured later 

    //==================================================

    //==================================================
    // Initialize I2C / screen 

#if NRF24L01_TEST_SCREEN 

    // Initialize the screen so it can be turned off 

    // Initialize I2C1 
    i2c_init(
        I2C1, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_backlight_off(); 

#endif   // NRF24L01_TEST_SCREEN 

    //==================================================

    //==================================================
    // Initialize SPI 

    // SPI for the RF module 
    spi_init(
        SPI2, 
        GPIOB,   // GPIO port for SCK pin 
        PIN_10,  // SCK pin 
        GPIOC,   // GPIO port for data (MISO/MOSI) pins 
        PIN_2,   // MISO pin 
        PIN_3,   // MOSI pin 
        SPI_BR_FPCLK_16, 
        SPI_CLOCK_MODE_0); 

    //==================================================

    //==================================================
    // Initialize test data 

    // Timing 
    nrf24l01_test.timer_nonblocking = TIM9; 
    nrf24l01_test.delay_timer.clk_freq = tim_get_pclk_freq(nrf24l01_test.timer_nonblocking); 
    nrf24l01_test.delay_timer.time_cnt_total = CLEAR; 
    nrf24l01_test.delay_timer.time_cnt = CLEAR; 
    nrf24l01_test.delay_timer.time_start = SET_BIT; 

    // Configuration 
    nrf24l01_test.pipe = NRF24L01_DP_1; 

    // Payload data 
    memset((void *)nrf24l01_test.read_buff, CLEAR, sizeof(nrf24l01_test.read_buff)); 

    //==================================================

    //==================================================
    // Initialize the device driver 

    NRF24L01_STATUS nrf24l01_init_status = NRF24L01_OK; 

    // General setup common to all device - must be called once during setup 
    nrf24l01_init_status |= nrf24l01_init(
        SPI2,                    // SPI port to use 
        GPIOC,                   // Slave select pin GPIO port 
        PIN_1,                   // Slave select pin number 
        GPIOC,                   // Enable pin (CE) GPIO port 
        PIN_0,                   // Enable pin (CE) number 
        TIM9,                    // General purpose timer port 
        NRF24L01_RF_FREQ,        // Initial RF channel frequency 
        NRF24L01_DR_2MBPS,       // Initial data rate to communicate at 
        NRF24L01_RF_PWR_0DBM);   // Initial power output to us 

    // Configure the PTX and PRX settings depending on the devices role/purpose. 
    nrf24l01_init_status |= nrf24l01_ptx_config(nrf24l01_pipe_addr); 
    nrf24l01_init_status |= nrf24l01_prx_config(nrf24l01_pipe_addr, nrf24l01_test.pipe); 

    // Power up the device now that it is configured 
    nrf24l01_init_status |= nrf24l01_pwr_up(); 

    // Check init status 
    if (nrf24l01_init_status)
    {
        uart_sendstring(USART2, "nRF24L01 init failed."); 
        while(1); 
    }
    else 
    {
        uart_sendstring(USART2, "nRF24L01 init success."); 
    }

    //==================================================

#if NRF24L01_HEARTBEAT 
    nrf24l01_heartbeat_test_init(); 
#elif NRF24L01_MANUAL_CONTROL 
    nrf24l01_manual_control_test_init(); 
#endif 
}

//=======================================================================================


//=======================================================================================
// Test code 

void nrf24l01_test_app(void)
{
#if NRF24L01_HEARTBEAT 
    nrf24l01_heartbeat_test_loop(); 
#elif NRF24L01_MANUAL_CONTROL 
    nrf24l01_manual_control_test_loop(); 
#endif 
}

//=======================================================================================


#if NRF24L01_HEARTBEAT 

//=======================================================================================
// Heartbeat test 

// Description 
// - Device 1 (master) sends a 'ping' to device 2 (slave) periodically 
// - Device 2 reads ping and responds with a confirmation 
// - Device 1 reads confirmation to verify communication is working 
// - If device 1 doesn't get a response then indicate a loss of communication 
// - Option to enter a "configuration" mode (on both devices) where settings can be 
//   manually changed by the user. Can then switch back to heartbeat mode to make sure 
//   there is still communication. 

//==================================================
// Macros 

#define HB_PERIOD 50000             // Time between checking for new data (us) 
#define HB_SEND_TIMER 40            // Counter to control message send frequency 

#define HB_LETTER_O 0x6F            // ASCII for lowercase "o" 
#define HB_EXCLAMATION 0x21         // ASCII for "!" 

//==================================================


//==================================================
// Variables 

#if NRF24L01_SYSTEM_1 

static const char 
hb_message[] = "ping%u", 
hb_response[] = "pong%u!"; 

#elif NRF24L01_SYSTEM_2 
#endif 

// Device tracker data record 
typedef struct hb_test_trackers_s 
{
    uint8_t send_timer; 
    uint8_t msg_counter; 

    char hb_msg[NRF24L01_MAX_PAYLOAD_LEN]; 
    char hb_res[NRF24L01_MAX_PAYLOAD_LEN]; 

    gpio_pin_state_t led_state; 
}
hb_test_trackers_t; 

// Device tracker instance 
static hb_test_trackers_t hb_test; 

//==================================================


//==================================================
// Setup 

void nrf24l01_heartbeat_test_init(void)
{
    hb_test.send_timer = CLEAR; 
    hb_test.msg_counter = CLEAR; 
    memset((void *)hb_test.hb_msg, CLEAR, sizeof(hb_test.hb_msg)); 
    memset((void *)hb_test.hb_res, CLEAR, sizeof(hb_test.hb_res)); 
    hb_test.led_state = GPIO_LOW; 

#if NRF24L01_SYSTEM_1 

    snprintf(hb_test.hb_msg, NRF24L01_MAX_PAYLOAD_LEN, hb_message, hb_test.msg_counter); 
    snprintf(hb_test.hb_res, NRF24L01_MAX_PAYLOAD_LEN, hb_response, hb_test.msg_counter); 

#elif NRF24L01_SYSTEM_2 
#endif 
}

//==================================================


//==================================================
// Loop 

void nrf24l01_heartbeat_test_loop(void)
{
    // Periodically check for action items 
    if (tim_compare(nrf24l01_test.timer_nonblocking, 
                    nrf24l01_test.delay_timer.clk_freq, 
                    HB_PERIOD, 
                    &nrf24l01_test.delay_timer.time_cnt_total, 
                    &nrf24l01_test.delay_timer.time_cnt, 
                    &nrf24l01_test.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

#if NRF24L01_SYSTEM_1 

        // Send heartbeat message periodically 
        if (!hb_test.send_timer++)
        {
            if (nrf24l01_send_payload((uint8_t *)hb_test.hb_msg) == NRF24L01_OK)
            {
                // Toggle the board LED 
                hb_test.led_state = GPIO_HIGH - hb_test.led_state; 
                gpio_write(GPIOA, GPIOX_PIN_5, hb_test.led_state); 

                // Show the heartbeat message 
                uart_send_new_line(USART2); 
                uart_sendstring(USART2, hb_test.hb_msg); 
                uart_sendstring(USART2, "... "); 
            }
        }

        if (hb_test.send_timer > HB_SEND_TIMER)
        {
            hb_test.send_timer = CLEAR; 
        }

        // Look for a heartbeat response 
        if (nrf24l01_data_ready_status() == nrf24l01_test.pipe)
        {
            nrf24l01_receive_payload(nrf24l01_test.read_buff); 

            // Check if the received message is the one we need 
            if (!strcmp((char *)nrf24l01_test.read_buff, hb_test.hb_res))
            {
                // Display the response and update the heatbeat message and response 
                uart_sendstring(USART2, (char *)nrf24l01_test.read_buff); 
                hb_test.msg_counter++; 
                snprintf(hb_test.hb_msg, NRF24L01_MAX_PAYLOAD_LEN, 
                         hb_message, hb_test.msg_counter); 
                snprintf(hb_test.hb_res, NRF24L01_MAX_PAYLOAD_LEN, 
                         hb_response, hb_test.msg_counter); 
            }
        }

#elif NRF24L01_SYSTEM_2 

        // Look for a heartbeat message 
        if (nrf24l01_data_ready_status() == nrf24l01_test.pipe)
        {
            nrf24l01_receive_payload(nrf24l01_test.read_buff); 

            // Modify the received message to get the desired response (changes "ping#" 
            // to "pong#!"). Note that the contents of the received message is not 
            // checked on purpose as it's not necessary for this test. 
            uint8_t msg_index = 1; 
            nrf24l01_test.read_buff[msg_index] = HB_LETTER_O; 
            
            while (nrf24l01_test.read_buff[msg_index] != NULL_CHAR) 
            {
                msg_index++; 
            }

            nrf24l01_test.read_buff[msg_index] = HB_EXCLAMATION; 
            nrf24l01_test.read_buff[++msg_index] = NULL_CHAR; 

            // Send the response back 
            nrf24l01_send_payload(nrf24l01_test.read_buff); 
        }

#endif 
    }
}

//==================================================

//=======================================================================================


#elif NRF24L01_MANUAL_CONTROL 

//=======================================================================================
// Manual control test 

// Description 
// - System 1: 
//   - Looks for user input at the serial terminal. If there is an input then try to 
//     match it to one of the pre-defined commands. If a command is matched then execute 
//     the command callback. If not then do nothing. 
//   - Commands are used to change device settings and ping the second system. 
//   - Periodically looks for a message from the second system. The second system will 
//     only send a message when responding to a ping. 
// - System 2: 
//   - Periodically check for a ping message from system 1. If a message is received then 
//     the message is checked to see if it matches the ping message. If there is a match 
//     then a response is sent. 

//==================================================
// Macros 

#define MC_PERIOD 50000   // Time between checking for new data (us) 

//==================================================


//==================================================
// Prototypes 

/**
 * @brief Send a ping and output the send operation status to the serial terminal 
 * 
 * @param arg_value : argument is not used 
 * @param arg_str : argument is not used 
 */
void nrf24l01_test_send_ping(
    uint8_t arg, 
    uint8_t *arg_str); 


/**
 * @brief RF channel set command 
 * 
 * @details "channel" command callback function. The value from the command updates the 
 *          RF channel that the device operates on but only if it's within range of the 
 *          available channels (0-125 MHz (input) + 2400 MHz). A value out of range will 
 *          not update the device. The status of the request is output for the user. 
 * 
 * @param rf_ch : RF channel to set the device to (0-125) 
 * @param arg_str : argument is not used 
 */
void nrf24l01_test_set_rf_ch(
    uint8_t rf_ch, 
    uint8_t *arg_str); 


/**
 * @brief RF data rate set command 
 * 
 * @details "data_rate" command callback function. The value from the command updates the 
 *          RF data rate that the device uses but only if it's within range of the 
 *          available data rates (see nrf24l01_data_rate_t). A value out of range will 
 *          not update the device. The status of the request is output for the user. 
 * 
 * @see nrf24l01_data_rate_t 
 * 
 * @param rf_dr : data rate to set the device to 
 * @param arg_str : argument is not used 
 */
void nrf24l01_test_set_rf_dr(
    uint8_t rf_dr, 
    uint8_t *arg_str); 


/**
 * @brief RF power output set command 
 * 
 * @details "power" command callback function. The value from the command updates the 
 *          devices' power ouput but only if it's within range of the available power 
 *          levels (see nrf24l01_rf_pwr_t). A value out of range will not update the 
 *          device. The status of the request is output for the user. 
 * 
 * @see nrf24l01_rf_pwr_t 
 * 
 * @param rf_pwr : power output to set the device to 
 * @param arg_str : argument is not used 
 */
void nrf24l01_test_set_rf_pwr(
    uint8_t rf_pwr, 
    uint8_t *arg_str); 


/**
 * @brief Choose which update message to display 
 * 
 * @param status : setting update status 
 */
void nrf24l01_test_update_feedback(uint8_t status); 


/**
 * @brief Display a message for the user in the serial terminal 
 */
void nrf24l01_test_user_feedback(const char *user_msg); 

//==================================================


//==================================================
// Variables 

// Messages sent between system 
static const char 
ping_msg[] = "ping",   // Doubles as a user command 
ping_res[] = "pong"; 


#if NRF24L01_SYSTEM_1 

// User commands and feedback 
static const char 
set_ch[] = "channel", 
set_dr[] = "data_rate", 
set_pwr[] = "power", 
update_success[] = "Success.", 
update_failed[] = "Failed.", 
invalid_value[] = "Invalid command value.", 
ping_failed[] = "Ping failed to send."; 

// Command table 
static const nrf24l01_cmds_t mc_cmd_table[NUM_USER_CMDS] = 
{
    {ping_msg, &nrf24l01_test_send_ping}, 
    {set_ch,   &nrf24l01_test_set_rf_ch}, 
    {set_dr,   &nrf24l01_test_set_rf_dr}, 
    {set_pwr,  &nrf24l01_test_set_rf_pwr} 
}; 

// Command data 
static nrf24l01_cmd_data_t mc_cmd_data; 

#endif 

//==================================================


//==================================================
// Setup 

void nrf24l01_manual_control_test_init(void)
{
#if NRF24L01_SYSTEM_1 

    //==================================================
    // UART DMA - further configure the UART for DMA input 

    // Enable the IDLE line interrupt 
    uart_interrupt_init(
        USART2, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 

    // Initialize the DMA stream for the UART 
    dma_stream_init(
        DMA1, 
        DMA1_Stream5, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // Configure the DMA stream for the UART 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)mc_cmd_data.cb, 
        (uint16_t)NRF24L01_TEST_MAX_INPUT); 

    // Enable the DMA stream for the UART 
    dma_stream_enable(DMA1_Stream5); 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 

    // Enable the interrupt handlers (called for each interrupt) - for USART2_RX 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0); 

    //==================================================

    memset((void *)mc_cmd_data.cb, CLEAR, sizeof(mc_cmd_data.cb)); 
    mc_cmd_data.cb_index = CLEAR; 
    memset((void *)mc_cmd_data.cmd_buff, CLEAR, sizeof(mc_cmd_data.cmd_buff)); 
    memset((void *)mc_cmd_data.cmd_id, CLEAR, sizeof(mc_cmd_data.cmd_id)); 
    mc_cmd_data.cmd_value = CLEAR; 
    memset((void *)mc_cmd_data.cmd_str, CLEAR, sizeof(mc_cmd_data.cmd_str)); 

    nrf24l01_test_user_prompt(); 
    
#elif NRF24L01_SYSTEM_2 
#endif 
}

//==================================================


//==================================================
// Loop 

void nrf24l01_manual_control_test_loop(void)
{
#if NRF24L01_SYSTEM_1 

    // Check for user input 
    nrf24l01_test_user_input(&mc_cmd_data, mc_cmd_table, NUM_USER_CMDS, NRF24L01_CMD_ARG_VALUE); 
    
#endif 

    // Periodically check for action items 
    if (tim_compare(nrf24l01_test.timer_nonblocking, 
                    nrf24l01_test.delay_timer.clk_freq, 
                    MC_PERIOD, 
                    &nrf24l01_test.delay_timer.time_cnt_total, 
                    &nrf24l01_test.delay_timer.time_cnt, 
                    &nrf24l01_test.delay_timer.time_start))
    {
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Look for a heartbeat message 
        if (nrf24l01_data_ready_status() == nrf24l01_test.pipe)
        {
            nrf24l01_receive_payload(nrf24l01_test.read_buff); 

#if NRF24L01_SYSTEM_1 

            // If a ping response message is seen then display the response 
            if (!strcmp(ping_res, (char *)nrf24l01_test.read_buff))
            {
                nrf24l01_test_user_feedback("\033[1A\033[1A\r"); 
                nrf24l01_test_user_feedback(ping_msg); 
                nrf24l01_test_user_feedback("..."); 
                nrf24l01_test_user_feedback(ping_res); 
                nrf24l01_test_user_prompt(); 
            }

#elif NRF24L01_SYSTEM_2 

            // If a ping message is seen then send a response back 
            if (!strcmp(ping_msg, (char *)nrf24l01_test.read_buff))
            {
                nrf24l01_send_payload((uint8_t *)ping_res);     
            }
#endif 
        }
    }
}

//==================================================


//==================================================
// Test functions 

#if NRF24L01_SYSTEM_1 

// PRX device connection status 
void nrf24l01_test_send_ping(
    uint8_t arg_value, 
    uint8_t *arg_str)
{
    if (nrf24l01_send_payload((uint8_t *)ping_msg) == NRF24L01_OK)
    {
        nrf24l01_test_user_feedback(ping_msg); 
        nrf24l01_test_user_feedback("..."); 
    }
    else 
    {
        nrf24l01_test_user_feedback(ping_failed); 
    }
}


// RF channel set callback 
void nrf24l01_test_set_rf_ch(
    uint8_t rf_ch, 
    uint8_t *arg_str)
{
    if (rf_ch <= NRF24L01_RF_CH_MAX)
    {
        nrf24l01_set_rf_ch(rf_ch); 
        nrf24l01_rf_ch_write(); 
        
        nrf24l01_rf_ch_read(); 
        nrf24l01_test_update_feedback(nrf24l01_get_rf_ch() == rf_ch); 
    }
    else 
    {
        nrf24l01_test_user_feedback(invalid_value); 
    }
}


// RF data rate set callback 
void nrf24l01_test_set_rf_dr(
    uint8_t rf_dr, 
    uint8_t *arg_str)
{
    if (rf_dr <= (uint8_t)NRF24L01_DR_250KBPS)
    {
        nrf24l01_set_rf_setup_dr((nrf24l01_data_rate_t)rf_dr); 
        nrf24l01_rf_setup_write(); 

        nrf24l01_rf_setup_read(); 
        nrf24l01_test_update_feedback(
            nrf24l01_get_rf_setup_dr() == (nrf24l01_data_rate_t)rf_dr); 
    }
    else 
    {
        nrf24l01_test_user_feedback(invalid_value); 
    }
}


// RF power output set callback 
void nrf24l01_test_set_rf_pwr(
    uint8_t rf_pwr, 
    uint8_t *arg_str)
{
    if (rf_pwr <= (uint8_t)NRF24L01_RF_PWR_0DBM)
    {
        nrf24l01_set_rf_setup_pwr((nrf24l01_rf_pwr_t)rf_pwr); 
        nrf24l01_rf_setup_write(); 

        nrf24l01_rf_setup_read(); 
        nrf24l01_test_update_feedback(
            nrf24l01_get_rf_setup_pwr() == (nrf24l01_rf_pwr_t)rf_pwr); 
    }
    else 
    {
        nrf24l01_test_user_feedback(invalid_value); 
    }
}


// Choose which update message to display 
void nrf24l01_test_update_feedback(uint8_t status)
{
    status ? nrf24l01_test_user_feedback(update_success) : 
             nrf24l01_test_user_feedback(update_failed); 
}


// Display a message for the user in the serial terminal 
void nrf24l01_test_user_feedback(const char *user_msg)
{
    uart_sendstring(USART2, user_msg); 
}

#endif 

//==================================================

//=======================================================================================

#endif 


//=======================================================================================
// Test functions 

// User terminal prompt 
void nrf24l01_test_user_prompt(void)
{
    uart_sendstring(USART2, "\r\n\n>>> "); 
}


// Check for user input and execute callbacks if a valid command arrives 
void nrf24l01_test_user_input(
    nrf24l01_cmd_data_t *cmd_data, 
    const nrf24l01_cmds_t *cmd_table, 
    uint8_t num_cmds, 
    nrf24l01_cmd_arg_t cmd_arg_type)
{
    // Check for user serial terminal input 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR; 

        // Copy the new contents in the circular buffer to the user input buffer 
        cb_parse(
            cmd_data->cb, 
            cmd_data->cmd_buff, 
            &cmd_data->cb_index, 
            NRF24L01_TEST_MAX_INPUT); 

        // Validate the input - parse into an ID and value if valid 
        if (nrf24l01_test_parse_cmd(cmd_data, cmd_arg_type))
        {
            // Valid input - compare the ID to each of the available pre-defined commands 
            for (uint8_t i = CLEAR; i < num_cmds; i++) 
            {
                if (!strcmp(cmd_table[i].user_cmds, (char *)cmd_data->cmd_id))
                {
                    // ID matched to a command. Execute the command. 
                    (cmd_table[i].cmd_ptr)(cmd_data->cmd_value, cmd_data->cmd_str); 
                    break; 
                }
            }
        }

        nrf24l01_test_user_prompt(); 
    }
}


// Parse the user input into an ID (command) and value 
uint8_t nrf24l01_test_parse_cmd(
    nrf24l01_cmd_data_t *cmd_data, 
    nrf24l01_cmd_arg_t cmd_arg_type)
{
    nrf24l01_cmd_arg_t cmd_arg_flag = NRF24L01_CMD_ARG_NONE; 
    uint8_t id_index = CLEAR; 
    uint8_t data = cmd_data->cmd_buff[0]; 
    uint8_t cmd_value[NRF24L01_TEST_MAX_INPUT]; 
    uint8_t value_size = CLEAR; 

    // Initialize data 
    memset((void *)cmd_data->cmd_id, CLEAR, sizeof(cmd_data->cmd_id)); 
    cmd_data->cmd_value = CLEAR; 
    memset((void *)cmd_data->cmd_str, CLEAR, sizeof(cmd_data->cmd_str)); 
    memset((void *)cmd_value, CLEAR, sizeof(cmd_value)); 

    // Parse the command into an ID and value 
    for (uint8_t i = CLEAR; data != NULL_CHAR; i++)
    {
        switch (cmd_arg_flag)
        {
            case NRF24L01_CMD_ARG_NONE:   // Command ID parsing 

                // Check that the command byte is within range 
                if ((data >= A_LO_CHAR && data <= Z_LO_CHAR) || 
                    (data >= A_UP_CHAR && data <= Z_UP_CHAR) || 
                    (data >= ZERO_CHAR && data <= NINE_CHAR) || 
                    (data == UNDERSCORE_CHAR))
                {
                    // Valid character byte seen 
                    cmd_data->cmd_id[i] = data; 
                }
                else if (data == SPACE_CHAR)
                {
                    // End of ID, start of optional value 
                    cmd_arg_flag = cmd_arg_type; 
                    cmd_data->cmd_id[i] = NULL_CHAR; 
                    id_index = i + 1; 
                }
                else 
                {
                    // Valid data not seen 
                    return FALSE; 
                }

                break; 

            case NRF24L01_CMD_ARG_VALUE:   // Command value parsing 

                if (data >= ZERO_CHAR && data <= NINE_CHAR)
                {
                    // Valid digit character byte seen 
                    cmd_value[i-id_index] = data; 
                    value_size++; 
                }
                else 
                {
                    // Valid data not seen 
                    return FALSE; 
                }

                break; 

            case NRF24L01_CMD_ARG_STR:   // Command string parsing 
                cmd_data->cmd_str[i-id_index] = data; 
                break; 

            default: 
                break; 
        }

        data = cmd_data->cmd_buff[i+1]; 
    }

    // Calculate the cmd value 
    for (uint8_t i = CLEAR; i < value_size; i++)
    {
        cmd_data->cmd_value += (uint8_t)char_to_int(cmd_value[i], value_size-i-1); 
    }

    return TRUE; 
}

//=======================================================================================
