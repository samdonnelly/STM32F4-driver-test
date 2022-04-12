/**
 * @file user_init.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once at the beginning of the program
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
    // Initialize timers 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // Initialize UART2
    uart2_init();

    // Initialize I2C1
    i2c1_init_master_mode();

    // Initialize the accelerometer 
    mpu6050_init();

    // Read the temperature from the accelerometer once 

    // Display the temperature data to the serial terminal
}
