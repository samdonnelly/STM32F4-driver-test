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

// Conditional compilation 
#define ADC_DMA_SECOND_CHANNEL 1      // Enables test code for second ADC channel 
#define ADC_DMA_THIRD_CHANNEL 1       // Enables test code for third ADC channel 
#define DMA_TEST_MODE_1 1             // Mode 1 test code control (see source file) 
#define DMA_TEST_MODE_2 0             // Mode 2 test code control (see source file) 
#define DMA_TEST_MODE_3 0             // Mode 3 test code control (see source file) 

// Data 
#define ADC_BUFF_SIZE 3               // Size according to the number of ADCs used 

// Formatting 
#define ADC_PRINT_SPACES 5            // Spaces between values displayed in the terminal 

//=======================================================================================


//=======================================================================================
// Enums 

// ADC index 
typedef enum {
    FIRST_ADC, 
    SECOND_ADC, 
    THIRD_ADC
} adc_index_t; 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief DMA test setup code 
 */
void dma_test_init(void); 


/**
 * @brief DMA test code 
 */
void dma_test_app(void); 

//=======================================================================================

#endif  // _DMA_TEST_H_ 
