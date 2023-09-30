/**
 * @file nrf24l01_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief nRF24L01 RF module test code 
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

//=======================================================================================


// Heartbeat 
// - Device 1 (master) sends a 'ping' to device 2 (slave) periodically 
// - Device 2 reads ping and responds with a confirmation 
// - Device 1 reads confirmation to verify communication is working 
// - If device 1 doesn't get a response then indicate a loss of communication 
// - Option to enter a "configuration" mode (on both devices) where settings can be 
//   manually changed by the user. Can then switch back to heartbeat mode to make sure 
//   there is still communication. 

// Multi-SPI 
// - Device 1 (master) sends a user input message to device 2 (slave) upon user request 
// - Device 2 reads message and attempts to save it to a file on an SD card 
// - Device 2 reports back the status of the SD card write to device 1 

// RC 
// - Device 1 (master) sends throttle command to device 2 (slave) 
// - Device 2 reads throttle command 
// - Throttle command goes to the ESC driver 


//=======================================================================================
// Function prototypes 

// RF channel set state 
void nrf24l01_test_rf_ch_state(void); 

// RF data rate set state 
void nrf24l01_test_rf_dr_state(void); 

// RF power output set state 
void nrf24l01_test_rf_pwr_state(void); 

// Heatbeat test - TX device state 
void nrf24l01_test_hb_tx_state(void); 

// Heatbeat test - RX device state 
void nrf24l01_test_hb_rx_state(void); 

//=======================================================================================


//=======================================================================================
// Global variables 

// Address sent by the PTX and address accepted by the PRX 
static uint8_t pipe_addr_buff[NRF24l01_ADDR_WIDTH] = {0xB3, 0xB4, 0xB5, 0xB6, 0x05}; 

// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Timing information 
    TIM_TypeDef *timer_nonblocking;   // Timer used for non-blocking delays 
    tim_compare_t delay_timer;        // Delay timing info 

    // User commands 
    uint8_t user_buff[NRF24L01_TEST_MAX_INPUT];  // Circular buffer for all user inputs 
    uint8_t cmd_buff[NRF24L01_TEST_MAX_INPUT];   // User command 
    uint8_t buff_index;                          // Circular buffer index 

    // Data 
    uint8_t hb_msg[NRF24L01_MAX_PAYLOAD_LEN];    // Heartbeat message 
    uint8_t read_buff[NRF24L01_MAX_PAYLOAD_LEN]; // Data read by PRX from PTX device 

    // 
    uint8_t cmd_index; 

    // Control flags 
    uint8_t state_progress : 1; 
    uint8_t new_param      : 1; 
    uint8_t first_pass     : 1; 
}
nrf24l01_test_trackers_t; 

// Device tracker instance 
static nrf24l01_test_trackers_t nrf24l01_test_data; 

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

// Command pointers 
typedef struct nrf24l01_user_cmds_s 
{
    char user_cmds[NRF24L01_TEST_MAX_INPUT]; 
    void (*nrf24l01_test_func_ptr)(void); 
}
nrf24l01_user_cmds_t; 


// User commands 
static nrf24l01_user_cmds_t cmd_table[NRF24L01_HB_NUM_STATES] = 
{
    {"ch_set",   &nrf24l01_test_rf_ch_state}, 
    {"dr_set",   &nrf24l01_test_rf_dr_state}, 
    {"pwr_set",  &nrf24l01_test_rf_pwr_state}, 
    {"run",      &nrf24l01_test_hb_tx_state} 
}; 

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 

#if NRF24L01_HEARTBEAT 

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 

//=======================================================================================


//=======================================================================================
// Setup code 

void nrf24l01_test_init(void)
{
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
        UART_DMA_ENABLE); 

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
    
    //==================================================

    //==================================================
    // Initialize DMA 

    // Initialize the DMA stream 
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

    // Configure the DMA stream 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)nrf24l01_test_data.user_buff, 
        (uint16_t)NRF24L01_TEST_MAX_INPUT); 

    // Enable the DMA stream 
    dma_stream_enable(DMA1_Stream5); 

    //==================================================

    //==================================================
    // Initialize interrupts 

    // Initialize interrupt handler flags (called once) 
    int_handler_init(); 

    // Enable the interrupt handlers (called for each interrupt) - for USART2_RX 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0); 

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

#if NRF24L01_MULTI_SPI && !NRF24L01_DEV1_CODE 

    // SPI2 and slave select pin for SD card 
    // This is on different pins than the RF module to test if the same SPI bus works across 
    // multiple pins. 
    spi_init(
        SPI2, 
        GPIOB,   // SCK pin GPIO port 
        PIN_10,  // SCK pin 
        GPIOB,   // Data (MISO/MOSI) pin GPIO port 
        PIN_14,  // MISO pin 
        PIN_15,  // MOSI pin 
        SPI_BR_FPCLK_16, 
        SPI_CLOCK_MODE_0); 
    spi_ss_init(GPIOB, PIN_12); 

#endif   // NRF24L01_MULTI_SPI && NRF24L01_DEV1_CODE 

    //==================================================

    //==================================================
    // Initialize the device driver 

    // General setup common to all device - must be called once during setup 
    nrf24l01_init(
        SPI2,    // SPI port to use 
        GPIOC,   // Slave select pin GPIO port 
        PIN_1,   // Slave select pin number 
        GPIOC,   // Enable pin (CE) GPIO port 
        PIN_0,   // Enable pin (CE) number 
        TIM9);   // General purpose timer port 

    // Set the devices initial communication parameters - can be updated as needed 
    nrf24l01_set_rf_channel(NRF24L01_RF_FREQ); 
    nrf24l01_set_rf_dr(NRF24L01_DR_2MBPS); 
    nrf24l01_set_rf_pwr(NRF24L01_RF_PWR_0DBM); 

    // Configure the PTX and PRX settings depending on the devices role/purpose 
#if NRF24L01_DEV1_CODE 
    nrf24l01_ptx_config(pipe_addr_buff); 
#else   // NRF24L01_DEV1_CODE 
    nrf24l01_prx_config(pipe_addr_buff, NRF24L01_DP_1); 
#endif   // NRF24L01_DEV1_CODE 

    //==================================================

    //==================================================
    // Initialize SD card --> for testing multiple SPI pins on the same SPI bus 

#if NRF24L01_MULTI_SPI && !NRF24L01_DEV1_CODE 

    // SD card user initialization 
    hw125_user_init(SPI2, GPIOB, GPIOX_PIN_12); 

#endif   // NRF24L01_MULTI_SPI 
    
    //==================================================

    //==================================================
    // GPIO 

#if NRF24L01_DEV1_CODE 
    // Board LED - on when logic low 
    gpio_pin_init(GPIOA, PIN_5, MODER_GPO, OTYPER_PP, OSPEEDR_HIGH, PUPDR_NO); 
#else   // NRF24L01_DEV1_CODE 
#endif   // NRF24L01_DEV1_CODE 

    //==================================================

    //==================================================
    // Variable initialization 

    // Timing information 
    nrf24l01_test_data.timer_nonblocking = TIM9; 
    nrf24l01_test_data.delay_timer.clk_freq = 
        tim_get_pclk_freq(nrf24l01_test_data.timer_nonblocking); 
    nrf24l01_test_data.delay_timer.time_cnt_total = CLEAR; 
    nrf24l01_test_data.delay_timer.time_cnt = CLEAR; 
    nrf24l01_test_data.delay_timer.time_start = SET_BIT; 

    // User commands 
    memset((void *)nrf24l01_test_data.user_buff, CLEAR, sizeof(nrf24l01_test_data.user_buff)); 
    memset((void *)nrf24l01_test_data.cmd_buff, CLEAR, sizeof(nrf24l01_test_data.cmd_buff)); 
    nrf24l01_test_data.buff_index = CLEAR; 

    // Payload data 
    memset((void *)nrf24l01_test_data.hb_msg, CLEAR, sizeof(nrf24l01_test_data.hb_msg)); 
    memset((void *)nrf24l01_test_data.read_buff, CLEAR, sizeof(nrf24l01_test_data.read_buff)); 
    strcpy((char *)nrf24l01_test_data.hb_msg, "ping"); 

    // 
    nrf24l01_test_data.state_progress = CLEAR_BIT; 
    nrf24l01_test_data.cmd_index = CLEAR; 

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    // 
#if NRF24L01_HEARTBEAT 

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 

    //==================================================

    uart_sendstring(USART2, "\r\n>>> "); 
}

//=======================================================================================


//=======================================================================================
// Test code 

void nrf24l01_test_app(void)
{
    // Check for user serial terminal input 
    if (handler_flags.usart2_flag)
    {
        // Reset the USART2 interrupt flag 
        handler_flags.usart2_flag = CLEAR; 

        // Copy the new contents in the circular buffer to the user input buffer 
        cb_parse(
            nrf24l01_test_data.user_buff, 
            nrf24l01_test_data.cmd_buff, 
            &nrf24l01_test_data.buff_index, 
            UART_TEST_MAX_INPUT); 

        // 
        if (!nrf24l01_test_data.state_progress)
        {
            // Loop through available commands to look for a match 
            for (uint8_t i = CLEAR; i < NRF24L01_HB_NUM_STATES; i++) 
            {
                if (str_compare((char *)nrf24l01_test_data.cmd_buff, cmd_table[i].user_cmds, BYTE_0)) 
                {
                    nrf24l01_test_data.cmd_index = i; 
                    nrf24l01_test_data.state_progress = SET_BIT; 
                    break; 
                }

                if (i == 3)
                {
                    uart_sendstring(USART2, "\r\nrun\r\n"); 
                    uart_sendstring(USART2, "\r\n>>> "); 
                }
            }
        }
    }

    // Go to the state function 
    (cmd_table[nrf24l01_test_data.cmd_index].nrf24l01_test_func_ptr)(); 

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    // 
#if NRF24L01_HEARTBEAT 

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 
}

//=======================================================================================


//=======================================================================================
// Test functions 

// RF channel set state 
void nrf24l01_test_rf_ch_state(void)
{
    // 
    nrf24l01_test_data.state_progress = CLEAR_BIT; 
    nrf24l01_test_data.cmd_index = SET_3; 
    uart_sendstring(USART2, "\r\nrf_ch\r\n"); 
    uart_sendstring(USART2, "\r\n>>> "); 

    // 
    if (nrf24l01_test_data.first_pass)
    {
        // Display prompt for the user and clear the first pass flag 
        uart_sendstring(USART2, "\r\nchannel: "); 
        nrf24l01_test_data.first_pass = CLEAR_BIT; 
    }
    else 
    {
        if (nrf24l01_test_data.new_param)
        {
            // Validate the input 
            if (1) // Change to data good 
            {
                // Data valid 
                // Update parameter 
                // Display successful update message 
            }
            else 
            {
                // Data not valid 
                // Display invalid input message 
            }

            // Reset control variables 
            nrf24l01_test_data.state_progress = CLEAR_BIT; 
            nrf24l01_test_data.first_pass = SET_BIT; 
            nrf24l01_test_data.cmd_index = CLEAR; 
        }
    }
}


// RF data rate set state 
void nrf24l01_test_rf_dr_state(void)
{
    // 
    nrf24l01_test_data.state_progress = CLEAR_BIT; 
    nrf24l01_test_data.cmd_index = SET_3; 
    uart_sendstring(USART2, "\r\nrf_dr\r\n"); 
    uart_sendstring(USART2, "\r\n>>> "); 
}


// RF power output set state 
void nrf24l01_test_rf_pwr_state(void)
{
    // 
    nrf24l01_test_data.state_progress = CLEAR_BIT; 
    nrf24l01_test_data.cmd_index = SET_3; 
    uart_sendstring(USART2, "\r\nrf_pwr\r\n"); 
    uart_sendstring(USART2, "\r\n>>> "); 
}


// Heatbeat test - TX device state 
void nrf24l01_test_hb_tx_state(void)
{
    // Local variables 
    static gpio_pin_state_t led_state = GPIO_LOW; 

    // Periodically ping the second device 
    if (tim_compare(nrf24l01_test_data.timer_nonblocking, 
                    nrf24l01_test_data.delay_timer.clk_freq, 
                    NRF24L01_HB_PERIOD, 
                    &nrf24l01_test_data.delay_timer.time_cnt_total, 
                    &nrf24l01_test_data.delay_timer.time_cnt, 
                    &nrf24l01_test_data.delay_timer.time_start))
    {
        // nrf24l01_test_data.delay_timer.time_start = SET_BIT; 

        // Try sending out a payload and toggle the led if it was sent 
        if (nrf24l01_send_payload(nrf24l01_test_data.hb_msg))
        {
            led_state = GPIO_HIGH - led_state; 
            gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
        } 
    }
}


// Heatbeat test - RX device state 
void nrf24l01_test_hb_rx_state(void)
{
    // Local variables 
    static uint16_t payload_count = CLEAR; 
    static uint16_t match_count = CLEAR; 
    uint8_t status_buff[NRF24L01_MAX_PAYLOAD_LEN]; 
    
    // Check if any data has been received 
    if (nrf24l01_data_ready_status(NRF24L01_DP_1))
    {
        payload_count++; 

        // Data has been received. Read the payload from the device RX FIFO. 
        nrf24l01_receive_payload(nrf24l01_test_data.read_buff, NRF24L01_DP_1); 

        // Compare the payload to the reference message to check for a match 
        if (str_compare((char *)nrf24l01_test_data.hb_msg, 
                        (char *)nrf24l01_test_data.read_buff, BYTE_1))
        {
            match_count++; 

            // Messages match. 
        }

        uart_sendstring(USART2, "\r"); 
        snprintf(
            (char *)status_buff, 
            NRF24L01_MAX_PAYLOAD_LEN, 
            "PL: %u, M: %u", 
            payload_count, 
            match_count); 
        uart_sendstring(USART2, (char *)status_buff); 

        // uart_sendstring(USART2, (char *)(&nrf24l01_test_data.read_buff[1])); 
        // uart_send_new_line(USART2); 
        memset((void *)nrf24l01_test_data.read_buff, CLEAR, 
               sizeof(nrf24l01_test_data.read_buff)); 
    }
}

//=======================================================================================
