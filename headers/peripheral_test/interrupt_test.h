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

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define INT_EXTI 1            // External interrupt code 
#define INT_ADC_ENABLE 0      // ADC interrupt code (EXTI0 must be included as well) 
#define INT_DMA_ENABLE 0      // DMA interrupt code (EXTI0 & ADC must be included as well) 

// Data 
#define INT_ADC_NUM_CONV 2    // Number of ADC conversions to keep track of 

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
