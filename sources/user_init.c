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
    uint16_t mpu6050_temp_sensor;
    uint8_t  mpu6050_temp_sensor_digits[5];

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

    // Read the temperature from the accelerometer once 
    mpu6050_temp_sensor = mpu6050_temp_read(MPU6050_1_ADDRESS);
    mpu6050_temp_sensor_digits[0] = (uint8_t)(
        ((mpu6050_temp_sensor % 100000) / 10000) + 48);
    mpu6050_temp_sensor_digits[1] = (uint8_t)(
        ((mpu6050_temp_sensor % 10000) / 1000) + 48);
    mpu6050_temp_sensor_digits[2] = (uint8_t)(
        ((mpu6050_temp_sensor % 1000) / 100) + 48);
    mpu6050_temp_sensor_digits[3] = (uint8_t)(
        ((mpu6050_temp_sensor % 100) / 10) + 48);
    mpu6050_temp_sensor_digits[4] = (uint8_t)(
        ((mpu6050_temp_sensor % 10) / 1) + 48);

    // Display the temperature data to the serial terminal
    uart2_sendstring("Temp Sensor Value = ");
    uart2_sendchar(mpu6050_temp_sensor_digits[0]);
    uart2_sendchar(mpu6050_temp_sensor_digits[1]);
    uart2_sendchar(mpu6050_temp_sensor_digits[2]);
    uart2_sendchar(mpu6050_temp_sensor_digits[3]);
    uart2_sendchar(mpu6050_temp_sensor_digits[4]);
    uart2_sendstring("\r\n");
}
