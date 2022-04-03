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

    // I2C1 init
    i2c1_init_master_mode();

    // wayintop LCD screen init. 
    hd44780u_init();
}
