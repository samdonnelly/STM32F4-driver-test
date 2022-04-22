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
    // Store the result of the accelerometer initialization 
    uint8_t  mpu6050_init_status;

    // Initialize timers 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // Initialize UART2
    uart2_init();

    // Initialize I2C1
    i2c1_init_master_mode();

    // Initialize the accelerometer 
    // The return value can be used to enter an error state
    mpu6050_init_status = mpu6050_init(MPU6050_1_ADDRESS);

    // Return the status of the accelerometer WHO_AM_I register 
    switch(mpu6050_init_status)
    {
        case TRUE:
            // 
            uart2_sendstring("Device seen\r\n");
            break;
        
        case FALSE:
            // 
            uart2_sendstring("Device not seen\r\n");
            break;

        default:
            break;
    }

    // Take initial measurements of the accelerometer to account for drift
    // This should be used in each application to ensure accurate analysis
}
