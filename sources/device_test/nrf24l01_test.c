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
#define NRF24L01_MANUAL_CONTROL 1          // SD card on same SPI bus but different pins 

// Hardware 
#define NRF24L01_TEST_SCREEN 0        // HD44780U screen in the system - shuts screen off 

//==================================================

// Configuration 
#define NRF24L01_RF_FREQ 10           // Comm frequency: 2400 MHz + this value (MHz) 

// User commands 
#define NRF24L01_TEST_MAX_INPUT 30    // Max user input command length (bytes) 
#define NRF24L01_NUM_USER_CMDS 4      // Number of test states 

#define MAX_INPUT_LEN 30 

//=======================================================================================


//=======================================================================================
// Enums 

// Test "states" 
typedef enum {
    NRF24L01_TEST_DEV1_HB_STATE, 
    NRF24L01_TEST_DEV1_MSPI_STATE, 
    NRF24L01_TEST_DEV1_RC_STATE, 
    NRF24L01_TEST_DEV2_HB_STATE, 
    NRF24L01_TEST_DEV2_MSPI_STATE, 
    NRF24L01_TEST_DEV2_RC_STATE 
} nrf24l01_test_states_t; 

//=======================================================================================


//=======================================================================================
// Global Variables 

// Command template 
typedef struct nrf24l01_cmds_s 
{
    char *user_cmds; 
    void (*cmd_ptr)(uint8_t); 
}
nrf24l01_cmds_t; 


// User command data 
typedef struct nrf24l01_cmd_data_s 
{
    uint8_t cb[MAX_INPUT_LEN];        // Circular buffer (CB) that stores user inputs 
    uint8_t cb_index;                 // CB index used for parsing commands 
    uint8_t cmd_buff[MAX_INPUT_LEN];  // Stores a user command parsed from the CB 
    uint8_t cmd_id[MAX_INPUT_LEN];    // Stores the ID of the user command 
    uint8_t cmd_value;                // Stores the value of the user command 
}
nrf24l01_cmd_data_t; 


// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Timing information 
    TIM_TypeDef *timer_nonblocking;                // Timer used for non-blocking delays 
    tim_compare_t delay_timer;                     // Delay timing info 

    // Configuration 
    nrf24l01_data_pipe_t pipe; 

    // User command data 
    // uint8_t user_buff[NRF24L01_TEST_MAX_INPUT];    // Circular buffer (CB) that stores user inputs 
    // uint8_t buff_index;                            // CB index used for parsing commands 
    // uint8_t cmd_buff[NRF24L01_TEST_MAX_INPUT];     // Stores a user command parsed from the CB 
    // uint8_t cmd_id[NRF24L01_TEST_MAX_INPUT];       // Stores the ID of the user command 
    // uint8_t cmd_value;                             // Stores the value of the user command 

    // Payload data 
    uint8_t hb_msg[NRF24L01_MAX_PAYLOAD_LEN];      // Heartbeat message 
    uint8_t read_buff[NRF24L01_MAX_PAYLOAD_LEN];   // Data read by PRX from PTX device 
    uint8_t write_buff[NRF24L01_MAX_PAYLOAD_LEN];  // Data sent to PRX from PTX device 

    // // Status 
    // uint8_t state;                                 // Test code "state" 
    // uint8_t conn_status;                           // Device connection status 
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
 * @brief User serial terminal input 
 * 
 * @param cmd_data 
 * @param cmd_table 
 */
void nrf24l01_test_user_input(
    nrf24l01_cmd_data_t *cmd_data, 
    nrf24l01_cmds_t *cmd_table); 


/**
 * @brief Parse the user command into an ID and value 
 * 
 * @details User commands are entered via the serial terminal in the format "<ID> <value>" 
 *          and this function will parse the input and store the results in the data 
 *          record if valid. This can then be used to loop for a matching command in the 
 *          'cmd_table' and call one of the command functions if there's a match. 
 * 
 * @see nrf24l01_test_rf_ch 
 * @see nrf24l01_test_rf_dr 
 * @see nrf24l01_test_rf_pwr 
 * @see nrf24l01_test_status 
 * 
//  * @param command_buffer : buffer that contains the unparsed user command 
 * @param cmd_data : user command data 
 * @return uint8_t : status of the parse - return true for a valid command 
 */
uint8_t nrf24l01_test_parse_cmd(nrf24l01_cmd_data_t *cmd_data); 


/**
 * @brief RF channel set command 
 * 
 * @details This function is called if an RF channel set command is sent by the user. 
 *          The value from the command updates the RF channel that the device operates 
 *          on. The user should check that the channel will be compatible between the 
 *          transmitting and receiving devices as this code will not verify that. If 
 *          the specified channel is out of range or the attempt to update the value is 
 *          unsuccessful then this will be communicated back over the serial terminal. 
 *          Available commands can be found in the 'cmd_table'. 
 * 
 * @param rf_ch : RF channel to set the device to 
 */
void nrf24l01_test_rf_ch(uint8_t rf_ch); 


/**
 * @brief RF data rate set command 
 * 
 * @details This function is called if an RF data rate set command is sent by the user. 
 *          The value from the command updates the RF data rate that the device uses. The 
 *          user should check that the data rate will be compatible between the 
 *          transmitting and receiving devices as this code will not verify that. If the 
 *          specified data rate is invalid or the attempt to update the value is 
 *          unsuccessful then this will be communicated back over the serial terminal. 
 *          Available commands can be found in the 'cmd_table'. 
 * 
 * @param rf_dr : data rate to set the device to 
 */
void nrf24l01_test_rf_dr(uint8_t rf_dr); 


/**
 * @brief RF power output set command 
 * 
 * @details This function is called if an RF power output set command is sent by the 
 *          user. The value from the command updates the devices power output. The user 
 *          should check that the power output works for their system and as this code 
 *          can't verify that. If the specified power output is invalid or the attempt 
 *          to update the value is unsuccessful then this will be communicated back over 
 *          the serial terminal. Available commands can be found in the 'cmd_table'. 
 * 
 * @param rf_pwr : power output to set the device to 
 */
void nrf24l01_test_rf_pwr(uint8_t rf_pwr); 


/**
 * @brief PRX device connection status command 
 * 
 * @details This function is called if a status command is sent by the user. This command 
 *          will output the connection status to the serial terminal. Connected means 
 *          there is communication happening between the two devices. Available commands 
 *          can be found in the 'cmd_table'. 
 * 
 * @param status : argument is not used - used for compatibility with function pointer 
 */
void nrf24l01_test_status(uint8_t status); 


/**
 * @brief Invalid user input serial terminal feedback 
 */
void nrf24l01_test_invalid_input(void); 

//=======================================================================================


//=======================================================================================
// User commands 

// Command pointers 
typedef struct nrf24l01_user_cmds_s 
{
    const char user_cmds[NRF24L01_TEST_MAX_INPUT]; 
    void (*nrf24l01_test_func_ptr)(uint8_t); 
    uint8_t cmd_mask; 
}
nrf24l01_user_cmds_t; 


// // Command table 
// static nrf24l01_user_cmds_t cmd_table[NRF24L01_NUM_USER_CMDS] = 
// {
//     {"rfch",   &nrf24l01_test_rf_ch,  0x3F}, 
//     {"rfdr",   &nrf24l01_test_rf_dr,  0x3F}, 
//     {"rfpwr",  &nrf24l01_test_rf_pwr, 0x3F}, 
//     {"status", &nrf24l01_test_status, 0x38} 
// }; 

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
        UART_DMA_ENABLE);   // 

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
    // Initialize test data 

    // Timing 
    nrf24l01_test.timer_nonblocking = TIM9; 
    nrf24l01_test.delay_timer.clk_freq = tim_get_pclk_freq(nrf24l01_test.timer_nonblocking); 
    nrf24l01_test.delay_timer.time_cnt_total = CLEAR; 
    nrf24l01_test.delay_timer.time_cnt = CLEAR; 
    nrf24l01_test.delay_timer.time_start = SET_BIT; 

    // Configuration 
    nrf24l01_test.pipe = NRF24L01_DP_1; 

    // User command data 
    // memset((void *)nrf24l01_test.user_buff, CLEAR, sizeof(nrf24l01_test.user_buff)); 
    // nrf24l01_test.buff_index = CLEAR; 
    // memset((void *)nrf24l01_test.cmd_buff, CLEAR, sizeof(nrf24l01_test.cmd_buff)); 
    // memset((void *)nrf24l01_test.cmd_id, CLEAR, sizeof(nrf24l01_test.cmd_id)); 
    // nrf24l01_test.cmd_value = CLEAR; 

    // Payload data 
    strcpy((char *)nrf24l01_test.hb_msg, "ping"); 
    memset((void *)nrf24l01_test.read_buff, CLEAR, sizeof(nrf24l01_test.read_buff)); 
    memset((void *)nrf24l01_test.write_buff, CLEAR, sizeof(nrf24l01_test.write_buff)); 

    // Status 
    // nrf24l01_test.conn_status = CLEAR_BIT; 

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
        uart_sendstring(USART2, "nRF24L01 init failed.\r\n"); 
        while(1); 
    }
    else 
    {
        uart_sendstring(USART2, "nRF24L01 init success.\r\n"); 
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
    // Universal (to all nrf24l01 tests) application test code 

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

    uint8_t rx_buff[NRF24L01_MAX_PAYLOAD_LEN]; 
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
    memset((void *)hb_test.rx_buff, CLEAR, sizeof(hb_test.rx_buff)); 
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
            nrf24l01_receive_payload(hb_test.rx_buff); 

            // Check if the received message is the one we need 
            if (!strcmp((char *)hb_test.rx_buff, hb_test.hb_res))
            {
                // Display the response and update the heatbeat message and response 
                uart_sendstring(USART2, (char *)hb_test.rx_buff); 
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
            nrf24l01_receive_payload(hb_test.rx_buff); 

            // Modify the received message to get the desired response (changes "ping#" 
            // to "pong#!"). Note that the contents of the received message is not 
            // checked on purpose as it's not necessary for this test. 
            uint8_t msg_index = 1; 
            hb_test.rx_buff[msg_index] = HB_LETTER_O; 
            
            while (hb_test.rx_buff[msg_index] != NULL_CHAR) 
            {
                msg_index++; 
            }

            hb_test.rx_buff[msg_index] = HB_EXCLAMATION; 
            hb_test.rx_buff[++msg_index] = NULL_CHAR; 

            // Send the response back 
            nrf24l01_send_payload(hb_test.rx_buff); 
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
//     the command. If not then do nothing. 
//   - Available commands will be to change device settings and ping the second system. 
//   - Periodically looks for a message from the second system. The second system will 
//     only send a message when responding to a ping. 
// - System 2: 
//   - Periodically check for a ping message from system 1. If a message is received then 
//     the message is checked to see if it matched the ping message. If there is a match 
//     then a response is sent. 

//==================================================
// Macros 

#define MC_PERIOD 100000             // Time between checking for new data (us) 

//==================================================


//==================================================
// Variables 

static const char 
ping_msg[] = "ping", 
ping_res[] = "pong", 
set_ch[] = "channel", 
set_dr[] = "data_rate", 
set_pwr[] = "power"; 


// Command table 
static const nrf24l01_cmds_t mc_cmd_table[] = 
{
    {ping_msg, &nrf24l01_test_rf_ch}, 
    {set_ch,   &nrf24l01_test_rf_dr}, 
    {set_dr,   &nrf24l01_test_rf_pwr}, 
    {set_pwr,  &nrf24l01_test_status} 
}; 


// 
static nrf24l01_cmd_data_t mc_cmd_data; 

//==================================================


//==================================================
// Prototypes 
//==================================================


//==================================================
// Setup 

void nrf24l01_manual_control_test_init(void)
{
#if NRF24L01_SYSTEM_1 

    //==================================================
    // UART DMA 

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

    // Provide an initial user prompt 
    uart_sendstring(USART2, "\r\n>>> "); 
    
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
    nrf24l01_test_user_input(&mc_cmd_data, mc_cmd_table); 
    
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

#if NRF24L01_SYSTEM_1 

        // 

#elif NRF24L01_SYSTEM_2 

        // Look for a heartbeat message 
        if (nrf24l01_data_ready_status() == nrf24l01_test.pipe)
        {
            nrf24l01_receive_payload(nrf24l01_test.read_buff); 

            // If a ping message is seen then send a response back 
            if (!strcmp(ping_msg, (char *)nrf24l01_test.read_buff))
            {
                nrf24l01_send_payload((uint8_t *)ping_res);     
            }
        }

#endif 
    }
}

//==================================================


//==================================================
// Test functions 

// RF channel set state 
void nrf24l01_test_rf_ch(uint8_t rf_ch)
{
    // Check that input is within bounds 
    if (rf_ch <= NRF24L01_RF_CH_MAX)
    {
        nrf24l01_set_rf_ch(rf_ch); 

        if (nrf24l01_get_rf_ch() == rf_ch)
        {
            uart_sendstring(USART2, "\r\nSuccess.\r\n"); 
        }
    }
    else 
    {
        nrf24l01_test_invalid_input(); 
    }
}


// RF data rate set state 
void nrf24l01_test_rf_dr(uint8_t rf_dr)
{
    // Check that input is within bounds 
    if (rf_dr <= (uint8_t)NRF24L01_DR_250KBPS)
    {
        nrf24l01_set_rf_setup_dr((nrf24l01_data_rate_t)rf_dr); 

        if (nrf24l01_get_rf_setup_dr() == (nrf24l01_data_rate_t)rf_dr)
        {
            uart_sendstring(USART2, "\r\nSuccess.\r\n"); 
        }
    }
    else 
    {
        nrf24l01_test_invalid_input(); 
    }
}


// RF power output set state 
void nrf24l01_test_rf_pwr(uint8_t rf_pwr)
{
    // Check that input is within bounds 
    if (rf_pwr <= (uint8_t)NRF24L01_RF_PWR_0DBM)
    {
        nrf24l01_set_rf_setup_pwr((nrf24l01_rf_pwr_t)rf_pwr); 

        if (nrf24l01_get_rf_setup_pwr() == (nrf24l01_rf_pwr_t)rf_pwr)
        {
            uart_sendstring(USART2, "\r\nSuccess.\r\n"); 
        }
    }
    else 
    {
        nrf24l01_test_invalid_input(); 
    }
}


// // PRX device connection status 
// void nrf24l01_test_status(uint8_t dummy_status)
// {
//     if (nrf24l01_test.conn_status)
//     {
//         uart_sendstring(USART2, "\r\nConnected.\r\n"); 
//     }
//     else 
//     {
//         uart_sendstring(USART2, "\r\nNot connected.\r\n"); 
//     }
// }


// Invalid input user feedback 
void nrf24l01_test_invalid_input(void)
{
    uart_sendstring(USART2, "\r\nInvalid cmd value.\r\n"); 
}

//==================================================

//=======================================================================================

#endif 


//=======================================================================================
// Test functions 

// User serial terminal input 
void nrf24l01_test_user_input(
    nrf24l01_cmd_data_t *cmd_data, 
    nrf24l01_cmds_t *cmd_table)
{
    // Check for user serial terminal input 
    if (handler_flags.usart2_flag)
    {
        // Reset the USART2 interrupt flag 
        handler_flags.usart2_flag = CLEAR; 

        // Copy the new contents in the circular buffer to the user input buffer 
        cb_parse(
            cmd_data->cb, 
            cmd_data->cmd_buff, 
            &cmd_data->cb_index, 
            MAX_INPUT_LEN); 

        // Validate the input - parse into an ID and value if valid 
        // if (nrf24l01_test_parse_cmd(cmd_data->cmd_buff))
        if (nrf24l01_test_parse_cmd(cmd_data))
        {
            // Valid input - compare the ID to each of the available pre-defined commands 
            for (uint8_t i = CLEAR; i < NRF24L01_NUM_USER_CMDS; i++) 
            {
                if (!strcmp(cmd_table[i].user_cmds, cmd_data->cmd_id))
                {
                    // ID matched to a command. Execute the command. 
                    (cmd_table[i].cmd_ptr)(cmd_data->cmd_value); 
                    break; 
                }
            }
        }

        uart_sendstring(USART2, "\r\n>>> "); 
    }

    //==================================================
    // Old 

    // // Check for user serial terminal input 
    // if (handler_flags.usart2_flag)
    // {
    //     // Reset the USART2 interrupt flag 
    //     handler_flags.usart2_flag = CLEAR; 

    //     // Copy the new contents in the circular buffer to the user input buffer 
    //     cb_parse(
    //         nrf24l01_test.user_buff, 
    //         nrf24l01_test.cmd_buff, 
    //         &nrf24l01_test.buff_index, 
    //         NRF24L01_TEST_MAX_INPUT); 

    //     // Validate the input - parse into an ID and value if valid 
    //     if (nrf24l01_test_parse_cmd(nrf24l01_test.cmd_buff))
    //     {
    //         // Valid input - compare the ID to each of the available pre-defined commands 
    //         for (uint8_t i = CLEAR; i < NRF24L01_NUM_USER_CMDS; i++) 
    //         {
    //             // Check that the command is available for the "state" before comparing it 
    //             // against the ID. 
    //             if (cmd_table[i].cmd_mask & (SET_BIT << nrf24l01_test.state))
    //             {
    //                 // Command available. Compare with the ID. 
    //                 if (str_compare(
    //                         cmd_table[i].user_cmds, 
    //                         (char *)nrf24l01_test.cmd_id, 
    //                         BYTE_0)) 
    //                 {
    //                     // ID matched to a command. Execute the command. 
    //                     (cmd_table[i].nrf24l01_test_func_ptr)(nrf24l01_test.cmd_value); 
    //                     break; 
    //                 }
    //             }
    //         }
    //     }

    //     uart_sendstring(USART2, "\r\n>>> "); 
    // }

    //==================================================
}


// Parse the user command into an ID and value 
uint8_t nrf24l01_test_parse_cmd(nrf24l01_cmd_data_t *cmd_data)
{
    // Local variables 
    uint8_t id_flag = SET_BIT; 
    uint8_t id_index = CLEAR; 
    uint8_t data = CLEAR; 
    uint8_t cmd_value[NRF24L01_TEST_MAX_INPUT]; 
    uint8_t value_size = CLEAR; 

    // Initialize data 
    memset((void *)cmd_data->cmd_id, CLEAR, sizeof(cmd_data->cmd_id)); 
    cmd_data->cmd_value = CLEAR; 
    memset((void *)cmd_value, CLEAR, sizeof(cmd_value)); 

    // Parse the command into an ID and value 
    for (uint8_t i = CLEAR; cmd_data->cmd_buff[i] != NULL_CHAR; i++)
    {
        data = cmd_data->cmd_buff[i]; 

        if (id_flag)
        {
            // cmd ID parsing 

            id_index = i; 

            // Check that the command byte is within range 
            if ((data >= A_LO_CHAR && data <= Z_LO_CHAR) || 
                (data >= A_UP_CHAR && data <= Z_UP_CHAR))
            {
                // Valid character byte seen 
                cmd_data->cmd_id[i] = data; 
            }
            else if (data >= ZERO_CHAR && data <= NINE_CHAR)
            {
                // Valid digit character byte seen 
                id_flag = CLEAR_BIT; 
                cmd_data->cmd_id[i] = NULL_CHAR; 
                cmd_value[i-id_index] = data; 
                value_size++; 
            }
            else 
            {
                // Valid data not seen 
                return FALSE; 
            }
        }
        else 
        {
            // cmd value parsing 

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
        }
    }

    // Calculate the cmd value 
    for (uint8_t i = CLEAR; i < value_size; i++)
    {
        cmd_data->cmd_value += (uint8_t)char_to_int(cmd_value[i], value_size-i-1); 
    }

    return TRUE; 

    //==================================================
    // Old 

    // // Initialize data 
    // memset((void *)nrf24l01_test.cmd_id, CLEAR, sizeof(nrf24l01_test.cmd_id)); 
    // nrf24l01_test.cmd_value = CLEAR; 
    // memset((void *)cmd_value, CLEAR, sizeof(cmd_value)); 

    // // Parse the command into an ID and value 
    // for (uint8_t i = CLEAR; command_buffer[i] != NULL_CHAR; i++)
    // {
    //     data = command_buffer[i]; 

    //     if (id_flag)
    //     {
    //         // cmd ID parsing 

    //         id_index = i; 

    //         // Check that the command byte is within range 
    //         if ((data >= A_LO_CHAR && data <= Z_LO_CHAR) || 
    //             (data >= A_UP_CHAR && data <= Z_UP_CHAR))
    //         {
    //             // Valid character byte seen 
    //             nrf24l01_test.cmd_id[i] = data; 
    //         }
    //         else if (data >= ZERO_CHAR && data <= NINE_CHAR)
    //         {
    //             // Valid digit character byte seen 
    //             id_flag = CLEAR_BIT; 
    //             nrf24l01_test.cmd_id[i] = NULL_CHAR; 
    //             cmd_value[i-id_index] = data; 
    //             value_size++; 
    //         }
    //         else 
    //         {
    //             // Valid data not seen 
    //             return FALSE; 
    //         }
    //     }
    //     else 
    //     {
    //         // cmd value parsing 

    //         if (data >= ZERO_CHAR && data <= NINE_CHAR)
    //         {
    //             // Valid digit character byte seen 
    //             cmd_value[i-id_index] = data; 
    //             value_size++; 
    //         }
    //         else 
    //         {
    //             // Valid data not seen 
    //             return FALSE; 
    //         }
    //     }
    // }

    // // Calculate the cmd value 
    // for (uint8_t i = CLEAR; i < value_size; i++)
    // {
    //     nrf24l01_test.cmd_value += (uint8_t)char_to_int(cmd_value[i], value_size-i-1); 
    // }

    // return TRUE; 
    
    //==================================================
}

//=======================================================================================
