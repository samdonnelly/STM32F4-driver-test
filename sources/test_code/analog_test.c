/**
 * @file analog_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Analog test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "analog_test.h"

//=======================================================================================


// Setup code
void analog_test_init()
{
    // Setup code for the analog_test here 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize the ADC port 
    adc_port_init(); 
} 


// Test code 
void analog_test_app()
{
    // Test code for the analog_test here 
}
