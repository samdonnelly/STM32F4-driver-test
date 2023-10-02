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

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

// Heatbeat test - TX device state 
void nrf24l01_test_hb_tx(void); 

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 

#if NRF24L01_HEARTBEAT 

// Heatbeat test - RX device state 
void nrf24l01_test_hb_rx(void); 

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 

// Parse the user command into an ID and value 
uint8_t nrf24l01_test_parse_cmd(void); 

// RF channel set state 
void nrf24l01_test_rf_ch(
    uint8_t rf_ch); 

// RF data rate set state 
void nrf24l01_test_rf_dr(
    uint8_t rf_dr); 

// RF power output set state 
void nrf24l01_test_rf_pwr(
    uint8_t rf_pwr); 

// Invalid input user feedback 
void nrf24l01_test_invalid_input(void); 

//=======================================================================================


//=======================================================================================
// Global variables 

// Address sent by the PTX and address accepted by the PRX 
static uint8_t pipe_addr_buff[NRF24l01_ADDR_WIDTH] = {0xB3, 0xB4, 0xB5, 0xB6, 0x05}; 

//==================================================
// Test data record 

// Device tracker data record 
typedef struct nrf24l01_test_trackers_s 
{
    // Timing information 
    TIM_TypeDef *timer_nonblocking;              // Timer used for non-blocking delays 
    tim_compare_t delay_timer;                   // Delay timing info 

    // User commands 
    uint8_t user_buff[NRF24L01_TEST_MAX_INPUT];  // Circular buffer (CB) that stores user inputs 
    uint8_t buff_index;                          // CB index used for parsing commands 
    uint8_t cmd_buff[NRF24L01_TEST_MAX_INPUT];   // Stores a user command parsed from the CB 
    uint8_t cmd_id[NRF24L01_TEST_MAX_INPUT];     // Stores the ID of the user command 
    uint8_t cmd_value;                           // Stores the value of the user command 

    // Serial terminal IO control 
    uint8_t new_data_flag;                       // Indicates new cmd available 
    uint8_t rx_idle;                             // RX read idle flag 

    // Payload buffers 
    uint8_t hb_msg[NRF24L01_MAX_PAYLOAD_LEN];    // Heartbeat message 
    uint8_t read_buff[NRF24L01_MAX_PAYLOAD_LEN]; // Data read by PRX from PTX device 
}
nrf24l01_test_trackers_t; 

// Device tracker instance 
static nrf24l01_test_trackers_t nrf24l01_test_data; 

//==================================================

//==================================================
// User commands 

// Command pointers 
typedef struct nrf24l01_user_cmds_s 
{
    char user_cmds[NRF24L01_TEST_MAX_INPUT]; 
    void (*nrf24l01_test_func_ptr)(uint8_t); 
}
nrf24l01_user_cmds_t; 


// User commands 
static nrf24l01_user_cmds_t cmd_table[NRF24L01_NUM_USER_CMDS] = 
{
    {"rfch",   &nrf24l01_test_rf_ch}, 
    {"rfdr",   &nrf24l01_test_rf_dr}, 
    {"rfpwr",  &nrf24l01_test_rf_pwr} 
}; 

//==================================================

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

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
    nrf24l01_set_rf_pwr(NRF24L01_RF_PWR_6DBM); 

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
    // Variable and user initialization 

    // Timing information 
    nrf24l01_test_data.timer_nonblocking = TIM9; 
    nrf24l01_test_data.delay_timer.clk_freq = 
        tim_get_pclk_freq(nrf24l01_test_data.timer_nonblocking); 
    nrf24l01_test_data.delay_timer.time_cnt_total = CLEAR; 
    nrf24l01_test_data.delay_timer.time_cnt = CLEAR; 
    nrf24l01_test_data.delay_timer.time_start = SET_BIT; 

    // User commands 
    memset((void *)nrf24l01_test_data.user_buff, CLEAR, sizeof(nrf24l01_test_data.user_buff)); 
    nrf24l01_test_data.buff_index = CLEAR; 
    memset((void *)nrf24l01_test_data.cmd_buff, CLEAR, sizeof(nrf24l01_test_data.cmd_buff)); 
    memset((void *)nrf24l01_test_data.cmd_id, CLEAR, sizeof(nrf24l01_test_data.cmd_id)); 
    nrf24l01_test_data.cmd_value = CLEAR; 

    // Serial terminal IO control 
    nrf24l01_test_data.new_data_flag = CLEAR_BIT; 
    nrf24l01_test_data.rx_idle = CLEAR_BIT; 

    // Payload data 
    memset((void *)nrf24l01_test_data.hb_msg, CLEAR, sizeof(nrf24l01_test_data.hb_msg)); 
    memset((void *)nrf24l01_test_data.read_buff, CLEAR, sizeof(nrf24l01_test_data.read_buff)); 
    strcpy((char *)nrf24l01_test_data.hb_msg, "ping"); 

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

    uart_sendstring(USART2, "\r\n>>> "); 

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

        // Validate the input - parse into an ID and value if valid 
        if (nrf24l01_test_parse_cmd())
        {
            // Valid input - compare ID to the available pre-defined commands. If there's 
            // a match then go to the command function. 
            nrf24l01_test_data.new_data_flag = SET_BIT; 
            for (uint8_t i = CLEAR; i < NRF24L01_NUM_USER_CMDS; i++) 
            {
                if (str_compare(
                        cmd_table[i].user_cmds, 
                        (char *)nrf24l01_test_data.cmd_id, 
                        BYTE_0)) 
                {
                    (cmd_table[i].nrf24l01_test_func_ptr)(nrf24l01_test_data.cmd_value); 
                    nrf24l01_test_data.new_data_flag = CLEAR_BIT; 
                    break; 
                }
            }
        }

        uart_sendstring(USART2, "\r\n>>> "); 
    }

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

    // Run the normal TX state 
    nrf24l01_test_hb_tx(); 

#elif NRF24L01_MULTI_SPI 
    // 
#elif NRF24L01_RC 
    // 
#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 
    // 
#if NRF24L01_HEARTBEAT 

    // If the new data flag is set then toggle the RX idle flag. The RX idle flag will control 
    // whether the normal RX state is run where data gets output to the serial terminal. If the 
    // RX idle flag is set then the state won't run and nothing will be output which will provide 
    // a chance to run config commands by the user. The new data flag gets set when there is a 
    // valid user input but it does not match one of the pre-defined commands. 
    if (nrf24l01_test_data.new_data_flag)
    {
        nrf24l01_test_data.rx_idle = SET_BIT - nrf24l01_test_data.rx_idle; 
        nrf24l01_test_data.new_data_flag = CLEAR_BIT; 

        if (!nrf24l01_test_data.rx_idle)
        {
            uart_send_new_line(USART2); 
        }
    }
    if (!nrf24l01_test_data.rx_idle)
    {
        nrf24l01_test_hb_rx(); 
    }

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

#if NRF24L01_DEV1_CODE   // Start of device 1 code 

#if NRF24L01_HEARTBEAT 

// Heatbeat test - TX device state 
void nrf24l01_test_hb_tx(void)
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
        // time_start flag does not need to be set again because this timer runs 
        // continuously. 

        // Try sending out a payload and toggle the led if it was sent 
        if (nrf24l01_send_payload(nrf24l01_test_data.hb_msg))
        {
            led_state = GPIO_HIGH - led_state; 
            gpio_write(GPIOA, GPIOX_PIN_5, led_state); 
        } 
    }
}

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#else   // NRF24L01_DEV1_CODE --> End of device 1 code, start of device 2 code 

#if NRF24L01_HEARTBEAT 

// TODO change this to not output the message and add a ststus command for the user 

// Heatbeat test - RX device state 
void nrf24l01_test_hb_rx(void)
{
    // Local variables 
    
    // Check if any data has been received 
    if (nrf24l01_data_ready_status(NRF24L01_DP_1))
    {
        // Data has been received. Read the payload from the device RX FIFO. 
        nrf24l01_receive_payload(nrf24l01_test_data.read_buff, NRF24L01_DP_1); 

        // 
        if (str_compare((char *)nrf24l01_test_data.hb_msg, 
                        (char *)nrf24l01_test_data.read_buff, 
                        BYTE_1))
        {
            uart_sendstring(USART2, (char *)(&nrf24l01_test_data.read_buff[1])); 
            uart_send_new_line(USART2); 
        }

        // uart_sendstring(USART2, (char *)(&nrf24l01_test_data.read_buff[1])); 
        // uart_send_new_line(USART2); 
        memset((void *)nrf24l01_test_data.read_buff, CLEAR, 
               sizeof(nrf24l01_test_data.read_buff)); 
    }
}

#elif NRF24L01_MULTI_SPI 

#elif NRF24L01_RC 

#endif   // NRF24L01_HEARTBEAT || NRF24L01_MULTI_SPI || NRF24L01_RC 

#endif   // NRF24L01_DEV1_CODE --> End of device 2 code 


// Parse the user command into an ID and value 
uint8_t nrf24l01_test_parse_cmd(void)
{
    // Local variables 
    uint8_t id_flag = SET_BIT; 
    uint8_t id_index = CLEAR; 
    uint8_t data = CLEAR; 
    uint8_t cmd_value[NRF24L01_TEST_MAX_INPUT]; 
    uint8_t value_size = CLEAR; 

    // Initialize data 
    memset((void *)nrf24l01_test_data.cmd_id, CLEAR, sizeof(nrf24l01_test_data.cmd_id)); 
    nrf24l01_test_data.cmd_value = CLEAR; 
    memset((void *)cmd_value, CLEAR, sizeof(cmd_value)); 

    // Parse the command into an ID and value 
    for (uint8_t i = CLEAR; nrf24l01_test_data.cmd_buff[i] != NULL_CHAR; i++)
    {
        data = nrf24l01_test_data.cmd_buff[i]; 

        if (id_flag)
        {
            // cmd ID parsing 

            id_index = i; 

            // Check that the command byte is within range 
            if (data >= A_LO_CHAR && data <= Z_LO_CHAR)
            {
                // Valid character byte seen 
                nrf24l01_test_data.cmd_id[i] = data; 
            }
            else if (data >= ZERO_CHAR && data <= NINE_CHAR)
            {
                // Valid digit character byte seen 
                id_flag = CLEAR_BIT; 
                nrf24l01_test_data.cmd_id[i] = NULL_CHAR; 
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
        nrf24l01_test_data.cmd_value += (uint8_t)char_to_int(cmd_value[i], value_size-i-1); 
    }

    return TRUE; 
}


// RF channel set state 
void nrf24l01_test_rf_ch(
    uint8_t rf_ch)
{
    // Check that input is within bounds 
    if (rf_ch <= NRF24L01_RF_CH_MAX)
    {
        nrf24l01_set_rf_channel(rf_ch); 
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
void nrf24l01_test_rf_dr(
    uint8_t rf_dr)
{
    // Check that input is within bounds 
    if (rf_dr <= (uint8_t)NRF24L01_DR_250KBPS)
    {
        nrf24l01_set_rf_dr((nrf24l01_data_rate_t)rf_dr); 
        if (nrf24l01_get_rf_dr() == (nrf24l01_data_rate_t)rf_dr)
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
void nrf24l01_test_rf_pwr(
    uint8_t rf_pwr)
{
    // Check that input is within bounds 
    if (rf_pwr <= (uint8_t)NRF24L01_RF_PWR_0DBM)
    {
        nrf24l01_set_rf_pwr((nrf24l01_rf_pwr_t)rf_pwr); 
        if (nrf24l01_get_rf_pwr() == (nrf24l01_rf_pwr_t)rf_pwr)
        {
            uart_sendstring(USART2, "\r\nSuccess.\r\n"); 
        }
    }
    else 
    {
        nrf24l01_test_invalid_input(); 
    }
}


// Invalid input user feedback 
void nrf24l01_test_invalid_input(void)
{
    uart_sendstring(USART2, "\r\nInvalid cmd value.\r\n"); 
}

//=======================================================================================
