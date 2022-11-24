/**
 * @file int_handlers.h
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

#ifndef _INT_HANDLERS_H_ 
#define _INT_HANDLERS_H_

//================================================================================
// Includes 

#include "includes_app.h"
#include "includes_drivers.h"

//================================================================================


//================================================================================
// Globals 

// Interrupt flag data record 
typedef struct int_handle_flags_s
{
    // EXTI interrupt flags 
    uint8_t exti0_flag : 1;                    // Line 0 
    uint8_t exti1_flag : 1;                    // Line 1 
    uint8_t exti2_flag : 1;                    // Line 2 
    uint8_t exti3_flag : 1;                    // Line 3 
    uint8_t exti4_flag : 1;                    // Line 4 

    // DMA1 interrupt flags 
    uint8_t dma1_0_flag : 1;                   // Stream 0 
    uint8_t dma1_1_flag : 1;                   // Stream 1 
    uint8_t dma1_2_flag : 1;                   // Stream 2 
    uint8_t dma1_3_flag : 1;                   // Stream 3 
    uint8_t dma1_4_flag : 1;                   // Stream 4 
    uint8_t dma1_5_flag : 1;                   // Stream 5 
    uint8_t dma1_6_flag : 1;                   // Stream 6 
    uint8_t dma1_7_flag : 1;                   // Stream 7 

    // DMA2 interrupt flags 
    uint8_t dma2_0_flag : 1;                   // Stream 0 
    uint8_t dma2_1_flag : 1;                   // Stream 1 
    uint8_t dma2_2_flag : 1;                   // Stream 2 
    uint8_t dma2_3_flag : 1;                   // Stream 3 
    uint8_t dma2_4_flag : 1;                   // Stream 4 
    uint8_t dma2_5_flag : 1;                   // Stream 5 
    uint8_t dma2_6_flag : 1;                   // Stream 6 
    uint8_t dma2_7_flag : 1;                   // Stream 7 

    // Timer interrupt flags 
    uint8_t tim1_brk_tim9_glbl_flag  : 1;       // TIM1 break + TIM9 global 
    uint8_t tim1_up_tim10_glbl_flag  : 1;       // TIM1 update + TIM10 global 
    uint8_t tim1_trg_tim11_glbl_flag : 1;       // TIM1 trigger, communication and global 
    uint8_t tim1_cc_flag             : 1;       // TIM1 capture compare 
    uint8_t tim2_glbl_flag           : 1;       // TIM2 global 
    uint8_t tim3_glbl_flag           : 1;       // TIM3 global 
    uint8_t tim4_glbl_flag           : 1;       // TIM4 global 
    uint8_t tim5_glbl_flag           : 1;       // TIM5 global 

    // ADC interrupt flags 
    uint8_t adc_flag: 1; 
} 
int_handle_flags_t;


// Instance of interrupt flag data record - defined for external use 
extern int_handle_flags_t handler_flags; 

//================================================================================


//================================================================================
// Initialization 

/**
 * @brief Interrupt handler flag initialization 
 * 
 * @details 
 * 
 */
void int_handler_init(void); 

//================================================================================


//================================================================================
// Handlers 

/**
 * @brief EXTI Line 0 
 * 
 * @details 
 * 
 */
void EXTI0_IRQHandler(void); 


/**
 * @brief EXTI Line 1 
 * 
 * @details 
 * 
 */
void EXTI1_IRQHandler(void); 


/**
 * @brief EXTI Line 2 
 * 
 * @details 
 * 
 */
void EXTI2_IRQHandler(void); 


/**
 * @brief EXTI Line 3 
 * 
 * @details 
 * 
 */
void EXTI3_IRQHandler(void); 


/**
 * @brief EXTI Line 4 
 * 
 * @details 
 * 
 */
void EXTI4_IRQHandler(void); 


/**
 * @brief DMA1 Stream 0 
 * 
 * @details 
 * 
 */
void DMA1_Stream0_IRQHandler(void); 


/**
 * @brief DMA1 Stream 1 
 * 
 * @details 
 * 
 */
void DMA1_Stream1_IRQHandler(void); 


/**
 * @brief DMA1 Stream 2 
 * 
 * @details 
 * 
 */
void DMA1_Stream2_IRQHandler(void); 


/**
 * @brief DMA1 Stream 3 
 * 
 * @details 
 * 
 */
void DMA1_Stream3_IRQHandler(void); 


/**
 * @brief DMA1 Stream 4 
 * 
 * @details 
 * 
 */
void DMA1_Stream4_IRQHandler(void); 


/**
 * @brief DMA1 Stream 5 
 * 
 * @details 
 * 
 */
void DMA1_Stream5_IRQHandler(void); 


/**
 * @brief DMA1 Stream 6 
 * 
 * @details 
 * 
 */
void DMA1_Stream6_IRQHandler(void); 


/**
 * @brief DMA1 Stream 7 
 * 
 * @details 
 * 
 */
void DMA1_Stream7_IRQHandler(void); 


/**
 * @brief DMA2 Stream 0 
 * 
 * @details 
 * 
 */
void DMA2_Stream0_IRQHandler(void); 


/**
 * @brief DMA2 Stream 1 
 * 
 * @details 
 * 
 */
void DMA2_Stream1_IRQHandler(void); 


/**
 * @brief DMA2 Stream 2 
 * 
 * @details 
 * 
 */
void DMA2_Stream2_IRQHandler(void); 


/**
 * @brief DMA2 Stream 3 
 * 
 * @details 
 * 
 */
void DMA2_Stream3_IRQHandler(void); 


/**
 * @brief DMA2 Stream 4 
 * 
 * @details 
 * 
 */
void DMA2_Stream4_IRQHandler(void); 


/**
 * @brief DMA2 Stream 5 
 * 
 * @details 
 * 
 */
void DMA2_Stream5_IRQHandler(void); 


/**
 * @brief DMA2 Stream 6 
 * 
 * @details 
 * 
 */
void DMA2_Stream6_IRQHandler(void); 


/**
 * @brief DMA2 Stream 7 
 * 
 * @details 
 * 
 */
void DMA2_Stream7_IRQHandler(void); 


/**
 * @brief Timer 1 break + timer 9 global 
 * 
 * @details 
 * 
 */
void TIM1_BRK_TIM9_IRQHandler(void); 


/**
 * @brief Timer 1 update + timer 10 global 
 * 
 * @details 
 * 
 */
void TIM1_UP_TIM10_IRQHandler(void); 


/**
 * @brief Timer 1 trigger and communication + global 
 * 
 * @details 
 * 
 */
void TIM1_TRG_COM_TIM11_IRQHandler(void); 


/**
 * @brief Timer 1 capture compare 
 * 
 * @details 
 * 
 */
void TIM1_CC_IRQHandler(void); 


/**
 * @brief Timer 2 
 * 
 * @details 
 * 
 */
void TIM2_IRQHandler(void); 


/**
 * @brief Timer 3
 * 
 * @details 
 * 
 */
void TIM3_IRQHandler(void); 


/**
 * @brief Timer 4
 * 
 * @details 
 * 
 */
void TIM4_IRQHandler(void); 


/**
 * @brief Timer 5
 * 
 * @details 
 * 
 */
void TIM5_IRQHandler(void); 


/**
 * @brief ADC1 
 * 
 * @details 
 * 
 */
void ADC_IRQHandler(void); 

//================================================================================


#endif   // _INT_HANDLERS_H_ 
