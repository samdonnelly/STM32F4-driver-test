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

    // TODO make a send number function to the uart driver 

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
    separate_digits(mpu6050_temp_sensor); 

    //==============================================================


    //==============================================================
    // Accelerometer 
    mpu6050_accel_read(MPU6050_1_ADDRESS, mpu6050_accel);

    // X-axis
    mpu6050_accel[ACCEL_X_AXIS] = (int16_t)(
            mpu6050_accel_x_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_X_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart2_sendstring("ax = ");
    separate_digits(mpu6050_accel[ACCEL_X_AXIS]); 

    // Y-axis 
    mpu6050_accel[ACCEL_Y_AXIS] = (int16_t)(
            mpu6050_accel_y_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_Y_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart2_sendstring("ay = ");
    separate_digits(mpu6050_accel[ACCEL_Y_AXIS]);

    // Z-axis 
    mpu6050_accel[ACCEL_Z_AXIS] = (int16_t)(
            mpu6050_accel_z_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_Z_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart2_sendstring("az = ");
    separate_digits(mpu6050_accel[ACCEL_Z_AXIS]);

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
    separate_digits(mpu6050_gyro[GYRO_X_AXIS]); 

    // Y-axis 
    mpu6050_gyro[GYRO_Y_AXIS] = (int16_t)(mpu6050_gyro_y_calc(
                                              MPU6050_1_ADDRESS, 
                                              mpu6050_gyro[GYRO_Y_AXIS],
                                              mpu6050_gyro_offset[GYRO_Y_AXIS]) * 
                                          NO_DECIMAL_SCALAR);
    
    uart2_sendstring("gy = ");
    separate_digits(mpu6050_gyro[GYRO_Y_AXIS]);

    // Z-axis 
    mpu6050_gyro[GYRO_Z_AXIS] = (int16_t)(mpu6050_gyro_z_calc(
                                              MPU6050_1_ADDRESS, 
                                              mpu6050_gyro[GYRO_Z_AXIS],
                                              mpu6050_gyro_offset[GYRO_Z_AXIS]) * 
                                          NO_DECIMAL_SCALAR);
    
    uart2_sendstring("gz = ");
    separate_digits(mpu6050_gyro[GYRO_Z_AXIS]);

    //==============================================================

    // Go to a new line in the serial terminal 
    print_new_line();

    // Delay 
    tim9_delay_ms(LOOP_DELAY);
}

//
void separate_digits(int16_t value_to_print)
{
    // 
    static uint8_t mpu6050_sensor_digits[UINT16_DEC_DIGITS];

    // Determine the sign of the number 
    if (value_to_print < 0)
    {
        mpu6050_sensor_digits[0] = NEGATIVE_SIGN;
        value_to_print = -(value_to_print);
    }
    else 
    {
        mpu6050_sensor_digits[0] = POSITIVE_SIGN;
    }

    // 
    mpu6050_sensor_digits[1] = (uint8_t)(
        ((value_to_print % REMAINDER_100000) / DIVIDE_10000) + CHAR_OFFSET);
    
    mpu6050_sensor_digits[2] = (uint8_t)(
        ((value_to_print % REMAINDER_10000)  / DIVIDE_1000)  + CHAR_OFFSET);
    
    mpu6050_sensor_digits[3] = (uint8_t)(
        ((value_to_print % REMAINDER_1000)   / DIVIDE_100)   + CHAR_OFFSET);
    
    mpu6050_sensor_digits[4] = (uint8_t)(
        ((value_to_print % REMAINDER_100)    / DIVIDE_10)    + CHAR_OFFSET);
    
    mpu6050_sensor_digits[5] = (uint8_t)(
        ((value_to_print % REMAINDER_10)     / DIVIDE_1)     + CHAR_OFFSET);
    
    // 
    print_to_serial(mpu6050_sensor_digits);
}

// 
void print_to_serial(uint8_t *print_values)
{
    // 
    for (uint8_t i = 0; i < UINT16_DEC_DIGITS; i++)
    {
        // 
        uart2_sendchar(*print_values);
        print_values++;
    }

    // 
    uart2_sendstring("  ");
}

// 
void print_new_line(void)
{
    uart2_sendstring("\r\n");
}
