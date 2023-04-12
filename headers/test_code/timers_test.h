/**
 * @file timers_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Timers test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TIMERS_TEST_H_
#define _TIMERS_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include "int_handlers.h"

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define TIM_PERIODIC 1               // Periodic interrupt code 
#define TIM_PERIODIC_COUNT 0         // Periodic interrupt counter printout code 
#define TIM_PWM_OUTPUT 0             // PWM output code (requires periodic interrupt code) 
#define TIM_WS2812 1                 // WS2812 device code 
#define TIM_SWITCH_1 1               // User switch 1 (requires periodic interrupt code) 
#define TIM_SWITCH_2 0               // User switch 2 (requires user switch 1 code) 
#define TIM_SWITCH_3 0               // User switch 3 (requires user switch 2 code) 
#define TIM_SWITCH_4 0               // User switch 4 (requires user switch 3 code) 
#define TIM_CLK_FREQ 0               // Code for checking system clock frequencies 
#define TIM_NON_BLOCKING 0           // Non-blocking delay code 

// PWM 
#define PWM_INCREMENT 20
#define PWM_ARR 100 

// Timing 
#define TIM_NO_BLOCK_DELAY 3000000   // (us) 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Timers setup code 
 */
void timers_test_init(void); 


/**
 * @brief Timers test code 
 */
void timers_test_app(void); 

//=======================================================================================

#endif  // _TIMERS_TEST_H_ 
