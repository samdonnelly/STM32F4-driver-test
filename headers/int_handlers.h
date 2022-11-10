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
    uint8_t exti0_flag;              // Line 0 
    uint8_t exti1_flag;              // Line 1 
    uint8_t exti2_flag;              // Line 2 
    uint8_t exti3_flag;              // Line 3 
    uint8_t exti4_flag;              // Line 4 

    // DMA1 interrupt flags 
    uint8_t dma1_0_flag;             // Stream 0 
    uint8_t dma1_1_flag;             // Stream 1 
    uint8_t dma1_2_flag;             // Stream 2 
    uint8_t dma1_3_flag;             // Stream 3 
    uint8_t dma1_4_flag;             // Stream 4 
    uint8_t dma1_5_flag;             // Stream 5 
    uint8_t dma1_6_flag;             // Stream 6 
    uint8_t dma1_7_flag;             // Stream 7 

    // DMA2 interrupt flags 
    uint8_t dma2_0_flag;             // Stream 0 
    uint8_t dma2_1_flag;             // Stream 1 
    uint8_t dma2_2_flag;             // Stream 2 
    uint8_t dma2_3_flag;             // Stream 3 
    uint8_t dma2_4_flag;             // Stream 4 
    uint8_t dma2_5_flag;             // Stream 5 
    uint8_t dma2_6_flag;             // Stream 6 
    uint8_t dma2_7_flag;             // Stream 7 

    // ADC interrupt flags 
    uint8_t adc_flag; 
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
 * @brief ADC1 
 * 
 * @details 
 * 
 */
void ADC_IRQHandler(void); 

//================================================================================


#endif   // _INT_HANDLERS_H_ 
