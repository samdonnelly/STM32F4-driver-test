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
#define ESC_PARAM_ID 1              // Parameter identification code - no driver just timers 
#define ESC_PERIOD 3000             // ESC PWM timer period (auto-reload register) 
#define ESC_SECOND_DEVICE 0         // Second device code 

// Parameters 
#define ESC_FWD_SPEED_LIM 1600      // Forward PWM pulse time limit (us) 
#define ESC_REV_SPEED_LIM 1400      // Reverse PWM pulse time limit (us) 

// User input 
#define ESC_INPUT_BUF_LEN 5         // User input buffer length 

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
