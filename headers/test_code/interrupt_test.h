/**
 * @file interrupt_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Interrupt test code 
 * 
 * @version 0.1
 * @date 2022-11-06
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _INTERRUPT_TEST_H_
#define _INTERRUPT_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include "int_handlers.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define INT_ADC_NUM_CONV 2    // Number of ADC conversions to keep track of 

// Conditional compilation 
#define INT_EXTI0 1           // EXTI0 code 
#define INT_EXTI1 0           // EXTI1 code (EXTI0 must be included as well) 
#define INT_EXTI2 0           // EXTI2 code (EXTI1 must be included as well) 
#define INT_EXTI3 0           // EXTI3 code (EXTI2 must be included as well) 
#define INT_EXTI4 0           // EXTI4 code (EXTI3 must be included as well) 
#define INT_ADC_ENABLE 0      // ADC interrupt code (EXTI0 must be included as well) 
#define INT_DMA_ENABLE 0      // DMA interrupt code (EXTI0 & ADC must be included as well) 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Interrupt test setup code 
 */
void int_test_init(void); 


/**
 * @brief Interrupt test application code 
 */
void int_test_app(void); 

//=======================================================================================

#endif   // _INTERRUPT_TEST_H_
