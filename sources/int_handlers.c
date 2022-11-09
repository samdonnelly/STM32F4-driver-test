/**
 * @file int_handlers.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt handlers 
 * 
 * @version 0.1
 * @date 2022-11-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//================================================================================
// Includes 

#include "int_handlers.h"

//================================================================================


//================================================================================
// Initialization 

// Interrupt handler flag initialization 
void int_handler_init(void)
{
    // EXTI interrupt flags 
    handler_flags.exti0_flag = 0; 
    handler_flags.exti0_flag = 0; 
    handler_flags.exti0_flag = 0; 
    handler_flags.exti0_flag = 0; 
    handler_flags.exti0_flag = 0; 

    // DMA1 interrupt flags 
    handler_flags.dma1_0_flag = 0; 
    handler_flags.dma1_1_flag = 0; 
    handler_flags.dma1_2_flag = 0; 
    handler_flags.dma1_3_flag = 0; 
    handler_flags.dma1_4_flag = 0; 
    handler_flags.dma1_5_flag = 0; 
    handler_flags.dma1_6_flag = 0; 
    handler_flags.dma1_7_flag = 0; 

    // DMA2 interrupt flags 
    handler_flags.dma2_0_flag = 0; 
    handler_flags.dma2_1_flag = 0; 
    handler_flags.dma2_2_flag = 0; 
    handler_flags.dma2_3_flag = 0; 
    handler_flags.dma2_4_flag = 0; 
    handler_flags.dma2_5_flag = 0; 
    handler_flags.dma2_6_flag = 0; 
    handler_flags.dma2_7_flag = 0; 

    // ADC interrupt flags 
    handler_flags.adc_flag = 0; 
}

//================================================================================


//================================================================================
// Handlers 

// External interrupt handler names are defined in startup_stm32f411xe.s 

// EXTI Line 0 
void EXTI0_IRQHandler(void)
{
    handler_flags.exti0_flag = SET_BIT; 
}


// EXTI Line 1 
void EXTI1_IRQHandler(void)
{
    handler_flags.exti1_flag = SET_BIT;  
}


// EXTI Line 2 
void EXTI2_IRQHandler(void)
{
    handler_flags.exti2_flag = SET_BIT;  
}


// EXTI Line 3 
void EXTI3_IRQHandler(void)
{
    handler_flags.exti3_flag = SET_BIT;  
}


// EXTI Line 4 
void EXTI4_IRQHandler(void)
{
    handler_flags.exti4_flag = SET_BIT;  
}


// DMA1 Stream 0 
void DMA1_Stream0_IRQHandler(void)
{
    handler_flags.dma1_0_flag = SET_BIT;  
}


// DMA1 Stream 1 
void DMA1_Stream1_IRQHandler(void)
{
    handler_flags.dma1_1_flag = SET_BIT;  
}


// DMA1 Stream 2 
void DMA1_Stream2_IRQHandler(void)
{
    handler_flags.dma1_2_flag = SET_BIT;  
}


// DMA1 Stream 3 
void DMA1_Stream3_IRQHandler(void)
{
    handler_flags.dma1_3_flag = SET_BIT;  
}


// DMA1 Stream 4 
void DMA1_Stream4_IRQHandler(void)
{
    handler_flags.dma1_4_flag = SET_BIT;  
}


// DMA1 Stream 5 
void DMA1_Stream5_IRQHandler(void)
{
    handler_flags.dma1_5_flag = SET_BIT;  
}


// DMA1 Stream 6 
void DMA1_Stream6_IRQHandler(void)
{
    handler_flags.dma1_6_flag = SET_BIT;  
}


// DMA1 Stream 7 
void DMA1_Stream7_IRQHandler(void)
{
    handler_flags.dma1_7_flag = SET_BIT;  
}


// DMA2 Stream 0 
void DMA2_Stream0_IRQHandler(void)
{
    handler_flags.dma2_0_flag = SET_BIT;  
}


// DMA2 Stream 1 
void DMA2_Stream1_IRQHandler(void)
{
    handler_flags.dma2_1_flag = SET_BIT;  
}


// DMA2 Stream 2 
void DMA2_Stream2_IRQHandler(void)
{
    handler_flags.dma2_2_flag = SET_BIT;  
}


// DMA2 Stream 3 
void DMA2_Stream3_IRQHandler(void)
{
    handler_flags.dma2_3_flag = SET_BIT;  
}


// DMA2 Stream 4 
void DMA2_Stream4_IRQHandler(void)
{
    handler_flags.dma2_4_flag = SET_BIT;  
}


// DMA2 Stream 5 
void DMA2_Stream5_IRQHandler(void)
{
    handler_flags.dma2_5_flag = SET_BIT;  
}


// DMA2 Stream 6 
void DMA2_Stream6_IRQHandler(void)
{
    handler_flags.dma2_6_flag = SET_BIT;  
}


// DMA2 Stream 7 
void DMA2_Stream7_IRQHandler(void)
{
    handler_flags.dma2_7_flag = SET_BIT;  
}


// ADC1 
void ADC_IRQHandler(void)
{
    handler_flags.adc_flag = SET_BIT;  
}

//================================================================================
