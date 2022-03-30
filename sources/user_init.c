/**
 * @file user_init.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief 
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "user_init.h"

//=======================================================================================


// User init function 
void user_init()
{
    // Initialize timer 9 based on the APB2 clock frequency 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // 0.1 second startup delay 
    tim9_delay_ms(STARTUP_DELAY);
}
