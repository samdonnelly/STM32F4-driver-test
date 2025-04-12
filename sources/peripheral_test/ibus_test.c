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
 *          - This code looks for data received via IBUS (UART) from the FlySky receiver. 
 *            Data is send very often (~7ms intervals) so a number of new data items are 
 *            checked to have accumulated before processing them. This is done because 
 *            systems using these receivers that do more than just use incoming receiver 
 *            data may not have time, or may not need to dedicate time, to processing 
 *            every new message. Once new data is processed, it's displayed to the serial 
 *            terminal so the user can see the value of each channel in the IBUS packet. 
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
// Includes 

#include "ibus_test.h" 

//=======================================================================================


//=======================================================================================
// Macros 
//=======================================================================================


//=======================================================================================
// Global data 
//=======================================================================================


//=======================================================================================
// Prototypes 
//=======================================================================================


//=======================================================================================
// Setup code 

void ibus_test_init(void)
{
    // 

    //==================================================
    // IBUS and UART 

    // UART2 init - Serial terminal 
    uart_init(
        hardware.user_uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR,                 // STOP bits 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // UART6 init - RC receiver 
    ibus_init(
        hardware.user_uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // UART6 interrupt init - RC receiver - IDLE line (RX) interrupts 
    uart_interrupt_init(
        rc_data.uart, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    //==================================================
}

//=======================================================================================


//=======================================================================================
// Test code 

void ibus_test_app(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Helper functions 
//=======================================================================================
