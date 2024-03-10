/**
 * @file esc_readytosky_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Readytosky ESC test code header 
 * 
 * @version 0.1
 * @date 2023-09-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _ESC_READYTOSKY_TEST_H_ 
#define _ESC_READYTOSKY_TEST_H_ 

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief ESC setup code 
 */
void esc_readytosky_test_init(void); 


/**
 * @brief ESC test code 
 */
void esc_readytosky_test_app(void); 

//=======================================================================================


//=======================================================================================
// Test functions 

/**
 * @brief ADC to ESC command mapping 
 * 
 * @details Takes the raw ADC value and converts it to a throttle command between -100% 
 *          (full reverse) and 100% (full forward). Note that there is a deadband in the 
 *          middle of the ADC values (100-155) that will produce a 0% throttle command. 
 *          
 *          NOTE: This mapping is done under the assumption that the ADC is 8-bits - i.e. 
 *                reads from 0-255. 
 * 
 * @param adc_val : ADC input reading (0-255) 
 * @return int16_t : mapped throttle command to be sent to the ESC driver 
 */
int16_t esc_test_adc_mapping(
    uint16_t adc_val); 

//=======================================================================================

#endif   // _ESC_READYTOSKY_TEST_H_ 
