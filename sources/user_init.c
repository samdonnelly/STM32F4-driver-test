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
    // Setup code here

    // UART2 init 
    uart2_init();

    // Verify that UART init is complete 
    uart2_sendstring("uart2 init complete\r\n");
    tim9_delay_ms(500);

    // I2C1 init
    i2c1_init_master_mode();

    // Verify that I2C init is complete 
    uart2_sendstring("i2c1 init complete\r\n");
    tim9_delay_ms(500);

    // wayintop LCD screen init. 
    hd44780u_init();
}
