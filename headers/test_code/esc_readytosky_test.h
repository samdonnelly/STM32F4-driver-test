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
// Macros 

// Conditional compilation 
#define ESC_CONTROLLER_MODE 1       // Code to control the ESC via a controller - knobs 
#define ESC_PARAM_ID 0              // Parameter identification code - no driver just timers 
#define ESC_SECOND_DEVICE 1         // Second device code 

// Parameters 
#define ESC_PERIOD 20000            // ESC PWM timer period (auto-reload register) 
#define ESC_FWD_SPEED_LIM 1600      // Forward PWM pulse time limit (us) 
#define ESC_REV_SPEED_LIM 1440      // Reverse PWM pulse time limit (us) 

// User input 
#define ESC_INPUT_BUF_LEN 15        // User input buffer length 
#define ESC_INPUT_MAX_LEN 5         // User input max length 
#define ESC_NUM_TEST_CMDS 2         // Number of user commands available 

// Data 
#define ESC_ADC_BUFF_LEN 2          // ADC buffer size 
#define ESC_ADC_REV_LIM 100         // ADC value reverse command limit 
#define ESC_ADC_FWD_LIM 155         // ADC value forward command limit 

//=======================================================================================


//=======================================================================================
// Function Prototypes 

/**
 * @brief Setup code 
 */
void esc_readytosky_test_init(void); 


/**
 * @brief Test code 
 */
void esc_readytosky_test_app(void); 

//=======================================================================================

#endif   // _ESC_READYTOSKY_TEST_H_ 
