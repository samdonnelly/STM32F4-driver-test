/**
 * @file sik_radio_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SiK radio driver test 
 * 
 * @details Test setup: 
 *          - Hardware: 
 *            * STM32F4 microcontroller with a serial connection to a PC 
 *            * Two SiK telemetry radios: 
 *              1. One connected to the STM32F4 via UART 
 *              2. Another connected to a PC running Mission Planner 
 *          - Software: 
 *            * Mission Planner on a PC with an established USB connection to one of the 
 *              telemetry radios. 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          - Notes: 
 *            * A single PC connected to the STM32F4 and running Mission Planner can be 
 *              used or it can be two separate computers. 
 *            * Certain communication (AT/RT) settings must match between both telemetry 
 *              radios in order for them to work together. See the SiK radio documentation 
 *              for more details. 
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
    #include "standard/mavlink.h" 
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
    uint8_t uart_dma_buff[SIK_TEST_MSG_BUFF_SIZE];     // Circular buffer for uart inputs 
    uint8_t data_buff[SIK_TEST_MSG_BUFF_SIZE];         // Stores latest uart input 
    uint8_t buff_index;                                // Circular buffer index 
}
sik_serial_data_t; 

static sik_serial_data_t user_data; 
static sik_serial_data_t radio_data; 


// Mavlink data 
typedef struct sik_mavlink_data_s 
{
    int channel; 
    mavlink_message_t msg; 
    char msg_buff[SIK_TEST_MSG_BUFF_SIZE];   // Should this be a circular buffer 
    uint16_t msg_buff_index; 
    mavlink_status_t status; 
}
sik_mavlink_data_t; 

static sik_mavlink_data_t mavlink_data; 

//=======================================================================================


//=======================================================================================
// Setup code 

void sik_radio_test_init(void)
{
    // Initialize data 
    memset((void *)user_data.uart_dma_buff, CLEAR, SIK_TEST_MSG_BUFF_SIZE); 
    memset((void *)user_data.data_buff, CLEAR, SIK_TEST_MSG_BUFF_SIZE); 
    user_data.buff_index = CLEAR; 
    memset((void *)radio_data.uart_dma_buff, CLEAR, SIK_TEST_MSG_BUFF_SIZE); 
    memset((void *)radio_data.data_buff, CLEAR, SIK_TEST_MSG_BUFF_SIZE); 
    radio_data.buff_index = CLEAR; 
    mavlink_data.channel = MAVLINK_COMM_0; 
    memset((void *)mavlink_data.msg_buff, CLEAR, SIK_TEST_MSG_BUFF_SIZE); 
    mavlink_data.msg_buff_index = CLEAR; 

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

    // UART1 interrupt init - SiK radio module - IDLE line interrupts 
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

    // UART2 interrupt init - Serial terminal - IDLE line interrupts 
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
        (uint32_t)radio_data.uart_dma_buff, 
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
        (uint32_t)user_data.uart_dma_buff, 
        (uint32_t)NULL, 
        (uint16_t)SIK_TEST_MSG_BUFF_SIZE); 

    // Enable DMA streams 
    dma_stream_enable(DMA1_Stream5);   // UART1 - Sik radio 
    dma_stream_enable(DMA2_Stream2);   // UART2 - Serial terminal 

    //==================================================
    
    //==================================================
    // Initialize interrupts 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(USART1_IRQn, EXTI_PRIORITY_0);   // UART1 - SiK radio 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0);   // UART2 - Serial terminal (user input) 

    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void sik_radio_test_app(void)
{
    // This test program reads data from both the SiK radio module and the serial 
    // terminal (user input). Both streams of data are connected to their own UART port. 
    // Both UART ports are configured to automatically store incoming data (RX line) 
    // in a data buffer using DMA. When the UART RX line goes IDLE after having data 
    // on it, it will trigger and interrupt which indicates new data has been fully 
    // received and is ready for processing. Data received from the SiK radio module 
    // will be processed and relayed to the serial terminal. Data received from the 
    // serial terminal will be processed and sent to the radio module as needed. 

    // Note that two radio modules are needed for this test to work. They don't have 
    // to be the same module but they must be able to communicate for data to be seen 
    // on this end. One module is connected to this controller and it must be either 
    // a generic SiK telemetry radio or an RFD900 modem as per the SiK radio driver 
    // being tested. It's recommended to have the other module connected to a device 
    // running Mission Planner. Doing this will simulate communication between a 
    // vehicle and ground station setup. The data that passes through this module 
    // (both incoming and outgoing) is assumed to be formatted following the MAVLINK 
    // protocol. For this reason, the mavlink v2 library is included and used. 

    // If the user inputs the prompt/command for AT command mode, then the device 
    // will enter AT command mode and stop relaying data from a remote radio module. 

    // New SiK radio module data received 
    if (handler_flags.usart1_flag)
    {
        handler_flags.usart1_flag = CLEAR_BIT; 

        // Parse the new radio message from the circular buffer to the data buffer 
        cb_parse(
            radio_data.uart_dma_buff, 
            radio_data.data_buff, 
            &radio_data.buff_index, 
            SIK_TEST_MSG_BUFF_SIZE); 

        // Loop until the mavlink library is done parsing 
        while (0)
        {
            // This does only a single byte at a time. 
            if (mavlink_parse_char(
                    mavlink_data.channel, 
                    mavlink_data.msg_buff[mavlink_data.msg_buff_index], 
                    &mavlink_data.msg, 
                    &mavlink_data.status))
            {
                // Message received 
                // Decode the message 
            }

            // When do we increment in the index? 
        }
    }


    // New serial terminal (user input) data received 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR_BIT; 

        // Check for AT command mode request 
        // Check for mavlink message to send 
    }
}

//=======================================================================================
