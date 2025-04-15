/**
 * @file ibus_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief IBUS test 
 * 
 * @details IBUS is a serial protocol by FlySky so their hardware is needed for this test. 
 *          
 *          Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC 
 *            * FlySky receiver with the IBUS pin(s) connected to a STM32F4 UART port. 
 *            * A device to wirelessly (radio) send channel data to the receiver (most 
 *              likely a FlySky transmitter). 
 *          - Software 
 *            * Serial monitor on a PC to allow the display of info from the STM32F4. 
 *          
 *          Configuration 
 *          - UART 
 *            * Two UART ports must be configured, one for the serial terminal connection 
 *              and another for the FlySky receiver. 
 *          - DMA 
 *            * DMA is configured for incoming data from the receiver since it will send 
 *              data whenever it's ready. This prevents and loss of data. 
 *          - Interrupts 
 *            * Interrupts are configured for the receiver UART port for when an RX line 
 *              is detected to have gone idle. This indicates that there was data being 
 *              received and that the data is now done being received. The test code 
 *              won't perform any actions unless and interrupt occurs. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            - This library provides an easy interface to allow data exchange with the 
 *              receiver using the IBUS, UART, DMA and interrupt drivers. 
 *          
 *          Procedure 
 *          - This code looks for data received via IBUS (UART) from the FlySky receiver 
 *            at a fixed interval (periodic interrupt). If data is there it will be read. 
 *            Every so often (but not every processing interval), the most recent IBUS 
 *            packet will be displayed for the user to see. IBUS data is sent frequently 
 *            (every ~7ms) and each packet could be used if needed but this test is meant 
 *            to simulate a system doing more than just looking for receiver data so the 
 *            receiver data is checked for at a fixed interval. More frequent sampling 
 *            can easily be done. 
 *          
 *          NOTE: IBUS data is send roughly every 7ms from the receiver. 
 * 
 * @version 0.1
 * @date 2025-04-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//=======================================================================================
// Testing Notes 

// - Check if the receiver sends data regardless of whether the transmitter is on or not. 

//=======================================================================================


//=======================================================================================
// Includes 

#include "ibus_test.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define IBUS_RC_BUFF_SIZE 500 
#define IBUS_SERIAL_BUFF_SIZE 100 
#define IBUS_TIMER_RELOAD 0x01F4    // == 500 
#define IBUS_DATA_DISPLAY_TIMER 5 

//=======================================================================================


//=======================================================================================
// Global data 

// IBUS test data 
typedef struct ibus_data_s 
{
    // Receiver input 
    uart_dma_input_cb_index_t rc; 
    uint8_t rc_cb[IBUS_RC_BUFF_SIZE];         // Circular buffer populated by DMA 
    uint8_t rc_data_in[IBUS_RC_BUFF_SIZE];    // Buffer that stores latest UART input 

    // IBUS packet handling 
    uint8_t packet_count; 
    uint8_t packets_index; 
    
    // Serial terminal output 
    USART_TypeDef *serial_uart; 
    char serial_data_out[IBUS_SERIAL_BUFF_SIZE]; 

}
ibus_data_t; 

static ibus_data_t ibus_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 
//=======================================================================================


//=======================================================================================
// Setup code 

void ibus_test_init(void)
{
    //==================================================
    // Data initialization 

    ibus_data.rc.uart = USART6; 
    ibus_data.rc.dma_stream = DMA2_Stream1; 
    ibus_data.rc.cb_index.cb_size = IBUS_RC_BUFF_SIZE; 
    ibus_data.rc.cb_index.head = CLEAR; 
    ibus_data.rc.cb_index.tail = CLEAR; 
    ibus_data.rc.dma_index.data_size = CLEAR; 
    ibus_data.rc.dma_index.ndt_old = dma_ndt_read(ibus_data.rc.dma_stream); 
    ibus_data.rc.dma_index.ndt_new = CLEAR; 
    ibus_data.rc.data_in_index = CLEAR; 
    memset((void *)ibus_data.rc_cb, CLEAR, sizeof(ibus_data.rc_cb)); 
    memset((void *)ibus_data.rc_data_in, CLEAR, sizeof(ibus_data.rc_data_in)); 

    ibus_data.packet_count = CLEAR; 
    ibus_data.packets_index = CLEAR; 

    ibus_data.serial_uart = USART2; 
    memset((void *)ibus_data.serial_data_out, CLEAR, sizeof(ibus_data.serial_data_out)); 

    //==================================================

    //==================================================
    // General 

    // Initialize GPIO ports 
    gpio_port_init(); 
    
    //==================================================

    //==================================================
    // Timers 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_100US_PSC, 
        IBUS_TIMER_RELOAD,   // ARR=500, (500 counts)*(100us/count) = 50ms 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM9); 

    //==================================================

    //==================================================
    // IBUS and UART 

    // UART2 init - Serial terminal 
    uart_init(
        ibus_data.serial_uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        CLEAR, 
        UART_FRAC_42_115200, 
        UART_MANT_42_115200, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // UART6 init (IBUS) - RC receiver 
    ibus_init(
        ibus_data.rc.uart, 
        GPIOA, 
        PIN_12, 
        PIN_11, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE); 

    // UART6 interrupt init - RC receiver - IDLE line (RX) interrupts 
    uart_interrupt_init(
        ibus_data.rc.uart, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    //==================================================

    //==================================================
    // DMA 

    // DMA2 stream init - UART6 - RC receiver 
    dma_stream_init(
        DMA2, 
        ibus_data.rc.dma_stream, 
        DMA_CHNL_5, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_HI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT, 
        DMA_ADDR_FIXED, 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE); 

    // DMA2 stream config - UART6 - RC receiver 
    dma_stream_config(
        ibus_data.rc.dma_stream, 
        (uint32_t)(&ibus_data.rc.uart->DR), 
        (uint32_t)ibus_data.rc_cb, 
        (uint32_t)NULL, 
        (uint16_t)IBUS_RC_BUFF_SIZE); 

    // Enable DMA streams 
    dma_stream_enable(ibus_data.rc.dma_stream);   // UART6 - RC receiver 

    //==================================================
    
    //==================================================
    // Initialize interrupts 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(USART6_IRQn, EXTI_PRIORITY_0);          // UART6 - RC receiver 
    nvic_config(TIM1_BRK_TIM9_IRQn, EXTI_PRIORITY_1);   // TIM9 - periodic interrupt 

    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void ibus_test_app(void)
{
    // Wait for the periodic interrupt before checking for new data. This is to simulate 
    // a system that will check for data at a fixed interval as opposed to polling for 
    // data or handling it as soon as an interrupt is triggered. 
    if (handler_flags.tim1_brk_tim9_glbl_flag)
    {
        handler_flags.tim1_brk_tim9_glbl_flag = CLEAR_BIT; 
        
        // Check to see if new data has arrived. In this case there should always be data 
        // ready by the time we check since the receivers sends new IBUS data every 7ms. 
        // The exceptions to this are if the periodic interrupt is configured to occur 
        // faster or the transmitter is not sending any data. We check anyway to be sure. 
        if (handler_flags.usart6_flag)
        {
            handler_flags.usart6_flag = CLEAR_BIT; 
            
            // Since this interrupt is handled at an interval as opposed to as soon as 
            // possible there is a chance the interrupt occurs while we're processing 
            // the new data. If this happens our packet count may not match the data 
            // collected so we make a local copy to keep it from changing. 
            uint8_t num_packets = ibus_data.packet_count; 
            ibus_data.packet_count = CLEAR; 
            
            // Parse the new receiver data from the circular buffer into the data buffer. 
            // The receiver will likely send multiple IBUS packets by the time we go to 
            // parse the data. As long as the buffer sizes are larger than the amount of 
            // data the receiver can send between periodic interrupt intervals then no 
            // data should be lost. 
            dma_cb_index(ibus_data.rc.dma_stream, &ibus_data.rc.dma_index, &ibus_data.rc.cb_index); 
            cb_parse(ibus_data.rc_cb, &ibus_data.rc.cb_index, ibus_data.rc_data_in); 
            
            // Only the most recent IBUS packet data is displayed for the user. The data 
            // display interval is also further divided because (a) the user doesn't need 
            // to see data updated as fast as the periodic interrupt and (b) because our 
            // buffer sizes would need to be larger for a longer periodic interrupt 
            // interval. 
            if (++ibus_data.packets_index >= IBUS_DATA_DISPLAY_TIMER)
            {
                ibus_data.packets_index = CLEAR; 
                ibus_packet_t *packet = 
                    (ibus_packet_t *)&ibus_data.rc_data_in[num_packets*IBUS_PACKET_BYTES]; 

                snprintf(
                    ibus_data.serial_data_out, 
                    IBUS_SERIAL_BUFF_SIZE, 
                    "\r%u %u %u %u %u %u %u %u %u %u %u %u %u %u", 
                    packet->items[IBUS_CH1], 
                    packet->items[IBUS_CH2], 
                    packet->items[IBUS_CH3], 
                    packet->items[IBUS_CH4], 
                    packet->items[IBUS_CH5], 
                    packet->items[IBUS_CH6], 
                    packet->items[IBUS_CH7], 
                    packet->items[IBUS_CH8], 
                    packet->items[IBUS_CH9], 
                    packet->items[IBUS_CH10], 
                    packet->items[IBUS_CH11], 
                    packet->items[IBUS_CH12], 
                    packet->items[IBUS_CH13], 
                    packet->items[IBUS_CH14]); 

                uart_send_str(ibus_data.serial_uart, ibus_data.serial_data_out); 
            }
        }
    }
}

//=======================================================================================


//=======================================================================================
// Interrupt override 

// USART6 - RC receiver IDLE line interrupts 
void USART6_IRQHandler(void)
{
    ibus_data.packet_count++; 

    handler_flags.usart6_flag = SET_BIT; 
    dummy_read(USART6->SR); 
    dummy_read(USART6->DR); 
}

//=======================================================================================
