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
