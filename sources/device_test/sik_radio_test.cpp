/**
 * @file sik_radio_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SiK radio driver test 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC 
 *            * Two SiK telemetry radios (or RFD900): 
 *              1. One connected to the STM32F4 
 *              2. Another connected to a PC running Mission Planner 
 *          - Software 
 *            * Mission Planner on a PC with an established USB connection to one of the 
 *              telemetry radios and using the MAVLINK protocol. 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          - Notes 
 *            * A single PC connected to the STM32F4 and running Mission Planner can be 
 *              used or it can be two separate computers. 
 *            * Certain communication (AT/RT) settings must match between both telemetry 
 *              radios in order for them to work together. See the SiK radio documentation 
 *              for more details. 
 *            * A connection between both radios must be established before any 
 *              meaningful work can be done by this code. 
 *          
 *          Configuration 
 *          - UART 
 *            * Two UART ports must be configured, one for the serial terminal connection 
 *              and another for the SiK radio. 
 *          - DMA 
 *            * Since data can be sent to the STM32F4 via UART from either the serial 
 *              terminal or the SiK radio without the STM32F4 requesting it, DMA is 
 *              configured for both UART ports so the data automatically gets stored 
 *              in a buffer. This prevents and loss of data. 
 *          - Interrupts 
 *            * Interrupts are configured for both UART ports for when an RX line is 
 *              detected to have gone idle. This indicates that there was data being 
 *              received and that the data is now done being received. The test code 
 *              won't perform any actions unless and interrupt occurs. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            - This library provides an easy interface to the SiK radio device driver 
 *              as well as UART, DMA and interrupt drivers. 
 *          - MAVLINK V2 C library 
 *            - The SiK radios and Mission Planner are designed to work with the MAVLINK 
 *              protocol. This code uses the library to decode and encode MAVLINK 
 *              messages. 
 *            - There are pre-built C and Python libraries but specific library versions 
 *              can be built manually. The pre-built C library works for this 
 *              application. 
 *          
 *          Procedure: 
 *          - This code looks for data received from both the SiK telemetry radio and 
 *            the serial terminal. If data is received by the radio then the code will 
 *            attempt to decode a pre-defined MAVLINK message. A valid message will be 
 *            sent to the serial terminal for the user to see. If data is received from 
 *            the serial terminal then the code will process the input and perform the 
 *            needed action which can be to either encode and send a MAVLINK message or 
 *            put the SiK radio into AT command mode. 
 * 
 * @version 0.1
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

//=======================================================================================
// Includes 

#include "sik_radio_test.h" 
#include "stm32f4xx_it.h" 

extern "C"
{
    // For C headers without C++ guards 
    #include "common/mavlink.h" 
}

//=======================================================================================


//=======================================================================================
// Macros 

#define SIK_TEST_MSG_BUFF_SIZE 200 

//=======================================================================================


//=======================================================================================
// Global data 

// User data 
typedef struct sik_serial_data_s 
{
    USART_TypeDef *uart; 
    DMA_TypeDef *dma_stream; 
    uint8_t cb[SIK_TEST_MSG_BUFF_SIZE];          // Circular buffer populated by DMA 
    cb_index_t cb_index;                         // Circular buffer indexing info 
    dma_index_t dma_index;                       // DMA transfer indexing info 
    uint8_t data_buff[SIK_TEST_MSG_BUFF_SIZE];   // Buffer that stores latest UART input 
}
sik_serial_data_t; 

static sik_serial_data_t radio_data; 
static sik_serial_data_t user_data; 


// Mavlink data 
typedef struct sik_mavlink_data_s 
{
    int channel; 
    mavlink_message_t msg; 
    uint16_t msg_buff_index; 
    mavlink_status_t status; 
}
sik_mavlink_data_t; 

static sik_mavlink_data_t mavlink_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief MAVLink message payload decode 
 * 
 * @details This function provides a means to decode any desired messages. Messages can 
 *          be added and removed as needed. A piece of code like this is needed for any 
 *          MAVLink application to define which messages to handle. 
 */
void sik_radio_test_mavlink_payload_decode(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

void sik_radio_test_init(void)
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    //==================================================
    // UART init 

    // UART1 init - SiK radio module 
    uart_init(
        USART1, 
        GPIOA, 
        PIN_10, 
        PIN_9, 
        UART_FRAC_84_115200, 
        UART_MANT_84_115200, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE); 

    // UART1 interrupt init - SiK radio module - IDLE line (RX) interrupts 
    uart_interrupt_init(
        USART1, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE, 
        UART_INT_ENABLE, 
        UART_INT_DISABLE, 
        UART_INT_DISABLE); 

    // UART2 init - Serial terminal 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_ENABLE); 

    // UART2 interrupt init - Serial terminal - IDLE line (RX) interrupts 
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
    // DMA 

    // DMA2 stream init - UART1 - SiK radio module 
    dma_stream_init(
        DMA2, 
        DMA2_Stream2, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_VHI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // DMA2 stream config - UART1 - SiK radio module 
    dma_stream_config(
        DMA2_Stream2, 
        (uint32_t)(&USART1->DR), 
        (uint32_t)radio_data.cb, 
        (uint32_t)NULL, 
        (uint16_t)SIK_TEST_MSG_BUFF_SIZE); 

    // DMA1 stream init - UART2 - Serial terminal 
    dma_stream_init(
        DMA1, 
        DMA1_Stream5, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_HI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // DMA1 stream config - UART2 - Serial terminal 
    dma_stream_config(
        DMA1_Stream5, 
        (uint32_t)(&USART2->DR), 
        (uint32_t)user_data.cb, 
        (uint32_t)NULL, 
        (uint16_t)SIK_TEST_MSG_BUFF_SIZE); 

    // Enable DMA streams 
    dma_stream_enable(DMA2_Stream2);   // UART1 - Sik radio 
    dma_stream_enable(DMA1_Stream5);   // UART2 - Serial terminal 

    //==================================================
    
    //==================================================
    // Initialize interrupts 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(USART1_IRQn, EXTI_PRIORITY_0);   // UART1 - SiK radio 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_1);   // UART2 - Serial terminal (user input) 

    //==================================================

    //==================================================
    // Initialize data 

    // Radio data 
    radio_data.uart = USART1; 
    radio_data.dma_stream = DMA2_Stream2; 
    memset((void *)radio_data.cb, CLEAR, sizeof(radio_data.cb)); 
    radio_data.cb_index.cb_size = SIK_TEST_MSG_BUFF_SIZE; 
    radio_data.cb_index.head = CLEAR; 
    radio_data.cb_index.tail = CLEAR; 
    radio_data.dma_index.data_size = CLEAR; 
    radio_data.dma_index.ndt_old = dma_ndt_read(radio_data.dma_stream); 
    radio_data.dma_index.ndt_new = CLEAR; 
    memset((void *)radio_data.data_buff, CLEAR, sizeof(radio_data.data_buff)); 

    // User data 
    user_data.uart = USART2; 
    user_data.dma_stream = DMA1_Stream5; 
    memset((void *)user_data.cb, CLEAR, sizeof(user_data.cb)); 
    user_data.cb_index.cb_size = SIK_TEST_MSG_BUFF_SIZE; 
    user_data.cb_index.head = CLEAR; 
    user_data.cb_index.tail = CLEAR; 
    user_data.dma_index.data_size = CLEAR; 
    user_data.dma_index.ndt_old = dma_ndt_read(user_data.dma_stream); 
    user_data.dma_index.ndt_new = CLEAR; 
    memset((void *)user_data.data_buff, CLEAR, sizeof(user_data.data_buff)); 

    // MAVLink data 
    mavlink_data.channel = MAVLINK_COMM_0; 
    mavlink_data.msg_buff_index = CLEAR; 

    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

// TODO 
// - Can multiple MAVLINK messages/packets come at once? 
// - Transition to AT command mode. 

void sik_radio_test_app(void)
{
    // New SiK radio module data received 
    if (handler_flags.usart1_flag)
    {
        handler_flags.usart1_flag = CLEAR_BIT; 
        mavlink_data.msg_buff_index = CLEAR; 

        // Parse the new radio data from the circular buffer into the data buffer. 
        dma_cb_index(radio_data.dma_stream, &radio_data.dma_index, &radio_data.cb_index); 
        cb_parse(radio_data.cb, &radio_data.cb_index, radio_data.data_buff); 

        // Look at each byte of the received data and try to decode MAVLink messages 
        // until there is no more data to check. 
        while (radio_data.data_buff[mavlink_data.msg_buff_index] != NULL_CHAR)
        {
            if (mavlink_parse_char(
                    mavlink_data.channel, 
                    radio_data.data_buff[mavlink_data.msg_buff_index++], 
                    &mavlink_data.msg, 
                    &mavlink_data.status))
            {
                // If a MAVLink message has been decoded then proceed to decode the 
                // message payload. This can happen more than once if multiple messages 
                // are in the received data. 
                sik_radio_test_mavlink_payload_decode(); 
            }
        }
    }


    // New serial terminal (user input) data received 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR_BIT; 

        // Parse the new user message from the circular buffer into the data buffer 
        dma_cb_index(user_data.dma_stream, &user_data.dma_index, &user_data.cb_index); 
        cb_parse(user_data.cb, &user_data.cb_index, user_data.data_buff); 

        // Check for AT command mode request 
        // Check for mavlink message to send 
    }
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// MAVLink message payload decode 
void sik_radio_test_mavlink_payload_decode(void)
{
    switch (mavlink_data.msg.msgid)
    {
        case MAVLINK_MSG_ID_HEARTBEAT: 
            mavlink_heartbeat_t heartbeat; 
            mavlink_msg_heartbeat_decode(
                &mavlink_data.msg, 
                &heartbeat); 
            break; 

        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: 
            mavlink_global_position_int_cov_t global_position; 
            mavlink_msg_global_position_int_cov_decode(
                &mavlink_data.msg, 
                &global_position); 
            break; 

        case MAVLINK_MSG_ID_GPS_STATUS: 
            break; 
        
        case MAVLINK_MSG_ID_COMMAND_INT: 
            // Call a separate function to decode mavlink_command_int_t.command 
            break; 
        
        case MAVLINK_MSG_ID_COMMAND_LONG: 
            // Call a separate function to decode mavlink_command_long_t.command 
            break; 
        
        default: 
            break; 
    }
}

//=======================================================================================
