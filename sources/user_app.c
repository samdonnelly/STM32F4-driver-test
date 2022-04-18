/**
 * @file user_app.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once per loop
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
    //==============================================================
    // Control Code 
    //  1. Read temperature once at the start (see uaer_init)
    //  2. Read accelerometer data once per loop (keep in mind max speed of sensor)
    //  3. Display the data over serial for viewing
    //  4. Delay 
    //  5. Repeat 
    //==============================================================

    // Local variables 
    static uint16_t mpu6050_temp_sensor;
    static uint8_t  mpu6050_temp_sensor_digits[5];
    
    // Read from the accelerometer

    // Read the temperature from the accelerometer once 
    // TODO make a send number function to the uart driver 
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

    // Delay 
    tim9_delay_ms(1000);
}
