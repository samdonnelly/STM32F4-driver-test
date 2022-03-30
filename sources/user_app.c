/**
 * @file user_app.c
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

#include "user_app.h"

//=======================================================================================


// User application 
void user_app()
{
    // Toggle on-board green LED (PA5)
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

    // 1 second delay between LED toggles 
    tim9_delay_ms(TOGGLE_DELAY);
}
