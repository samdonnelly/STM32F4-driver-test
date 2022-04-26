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
    //  1. Read raw temperature feedback, format it into degC, print it to serial 
    //  2. Read raw acceleromater feedback, format it into g's, print it to serial 
    //  3. Read raw gyroscope feedback, format it into deg/s, print it to serial 
    //  4. Start a new serial terminal line 
    //  5. Delay 
    //  6. Repeat 
    //==============================================================

    // Local variables 
    static int16_t mpu6050_temp_sensor;
    static int16_t mpu6050_accel[3];
    static int16_t mpu6050_accel_offset[3];
    static int16_t mpu6050_gyro[3];
    static int16_t mpu6050_gyro_offset[3];
    static uint8_t one_time = 0;

    // Run once 
    if (one_time == 0)
    {
        mpu6050_calibrate(
            MPU6050_1_ADDRESS, 
            mpu6050_accel_offset,
            mpu6050_gyro_offset);
        
        one_time++;
    }

    //==============================================================
    // Temperature 
    mpu6050_temp_sensor = mpu6050_temp_read(MPU6050_1_ADDRESS);
    mpu6050_temp_sensor = (int16_t)(mpu6050_temp_calc(mpu6050_temp_sensor) * 
                                    NO_DECIMAL_SCALAR);
    
    uart2_sendstring("temp = ");
    uart2_send_integer(mpu6050_temp_sensor);
    uart2_send_spaces(UART2_2_SPACES);

    //==============================================================


    //==============================================================
    // Accelerometer 
    mpu6050_accel_read(MPU6050_1_ADDRESS, mpu6050_accel);

    // X-axis
    mpu6050_accel[ACCEL_X_AXIS] = (int16_t)(
            mpu6050_accel_x_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_X_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart2_sendstring("ax = ");
    uart2_send_integer(mpu6050_accel[ACCEL_X_AXIS]); 
    uart2_send_spaces(UART2_2_SPACES);

    // Y-axis 
    mpu6050_accel[ACCEL_Y_AXIS] = (int16_t)(
            mpu6050_accel_y_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_Y_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart2_sendstring("ay = ");
    uart2_send_integer(mpu6050_accel[ACCEL_Y_AXIS]);
    uart2_send_spaces(UART2_2_SPACES);

    // Z-axis 
    mpu6050_accel[ACCEL_Z_AXIS] = (int16_t)(
            mpu6050_accel_z_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_Z_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart2_sendstring("az = ");
    uart2_send_integer(mpu6050_accel[ACCEL_Z_AXIS]);
    uart2_send_spaces(UART2_2_SPACES);

    //==============================================================


    //==============================================================
    // Gyroscope 
    mpu6050_gyro_read(MPU6050_1_ADDRESS, mpu6050_gyro);

    // X-axis
    mpu6050_gyro[GYRO_X_AXIS] = (int16_t)(mpu6050_gyro_x_calc(
                                              MPU6050_1_ADDRESS, 
                                              mpu6050_gyro[GYRO_X_AXIS],
                                              mpu6050_gyro_offset[GYRO_X_AXIS]) * 
                                          NO_DECIMAL_SCALAR);
    
    uart2_sendstring("gx = ");
    uart2_send_integer(mpu6050_gyro[GYRO_X_AXIS]); 
    uart2_send_spaces(UART2_2_SPACES);

    // Y-axis 
    mpu6050_gyro[GYRO_Y_AXIS] = (int16_t)(mpu6050_gyro_y_calc(
                                              MPU6050_1_ADDRESS, 
                                              mpu6050_gyro[GYRO_Y_AXIS],
                                              mpu6050_gyro_offset[GYRO_Y_AXIS]) * 
                                          NO_DECIMAL_SCALAR);
    
    uart2_sendstring("gy = ");
    uart2_send_integer(mpu6050_gyro[GYRO_Y_AXIS]);
    uart2_send_spaces(UART2_2_SPACES);

    // Z-axis 
    mpu6050_gyro[GYRO_Z_AXIS] = (int16_t)(mpu6050_gyro_z_calc(
                                              MPU6050_1_ADDRESS, 
                                              mpu6050_gyro[GYRO_Z_AXIS],
                                              mpu6050_gyro_offset[GYRO_Z_AXIS]) * 
                                          NO_DECIMAL_SCALAR);
    
    uart2_sendstring("gz = ");
    uart2_send_integer(mpu6050_gyro[GYRO_Z_AXIS]);
    uart2_send_spaces(UART2_2_SPACES);

    //==============================================================

    // Go to a new line in the serial terminal 
    uart2_send_new_line();

    // Delay 
    tim9_delay_ms(LOOP_DELAY);
}
