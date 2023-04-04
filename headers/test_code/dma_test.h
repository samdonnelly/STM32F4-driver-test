/**
 * @file dma_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief DMA test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _DMA_TEST_H_
#define _DMA_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define ADC_DMA_SECOND_CHANNEL 0      // Enables test code for second ADC channel 
#define DMA_TEST_MODE_1 0             // Mode 1 test code control (see source file) 
#define DMA_TEST_MODE_2 0             // Mode 2 test code control (see source file) 
#define DMA_TEST_MODE_3 0             // Mode 3 test code control (see source file) 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief DMA test setup code 
 * 
 */
void dma_test_init(void); 


/**
 * @brief DMA test code 
 * 
 */
void dma_test_app(void); 

//=======================================================================================

#endif  // _DMA_TEST_H_ 
