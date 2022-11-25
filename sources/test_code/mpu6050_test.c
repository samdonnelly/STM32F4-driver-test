/**
 * @file mpu6050_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 test code 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "mpu6050_test.h"

//=======================================================================================


// Setup code
void mpu6050_test_init()
{
    // Store the result of the accelerometer initialization 
    uint8_t mpu6050_init_status;

    // Store the results of the accelerometer self-test 
    uint8_t mpu_self_test_result;

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIMERS_APB2_84MHZ_1US_PRESCALAR, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART2
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);
    uart_sendstring(USART2, "test\r\n"); 

    // Initialize I2C1
    i2c1_init(
        I2C1, 
        I2C1_SDA_PB9,
        I2C1_SCL_PB8,
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);

    // Initialize the accelerometer 
    // The return value can be used to enter an error state
    mpu6050_init_status = mpu6050_init(
                            MPU6050_1_ADDRESS,
                            DLPF_CFG_1,
                            SMPLRT_DIV_0,
                            AFS_SEL_4,
                            FS_SEL_500);

    // Return the status of the accelerometer WHO_AM_I register 
    switch(mpu6050_init_status)
    {
        case TRUE:
            // 
            uart_sendstring(USART2, "Device seen\r\n");
            break;
        
        case FALSE:
            // 
            uart_sendstring(USART2, "Device not seen\r\n");
            break;

        default:
            break;
    }

    // MPU6050 self-test 
    mpu_self_test_result = mpu6050_self_test(MPU6050_1_ADDRESS);
    uart_sendstring(USART2, "MPU6050 Self-Test Result = ");
    uart_send_integer(USART2, (int16_t)(mpu_self_test_result));
    uart_send_new_line(USART2);
} 


// Test code 
void mpu6050_test_app()
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
    static int16_t mpu6050_accel[MPU6050_NUM_ACCEL_AXIS];
    static int16_t mpu6050_accel_offset[MPU6050_NUM_ACCEL_AXIS];
    static int16_t mpu6050_gyro[MPU6050_NUM_GYRO_AXIS];
    static int16_t mpu6050_gyro_offset[MPU6050_NUM_GYRO_AXIS];
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
    
    uart_sendstring(USART2, "temp = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, UART2_2_SPACES);

    //==============================================================


    //==============================================================
    // Accelerometer 
    mpu6050_accel_read(MPU6050_1_ADDRESS, mpu6050_accel);

    // X-axis
    mpu6050_accel[ACCEL_X_AXIS] = (int16_t)(
            mpu6050_accel_x_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_X_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "ax = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Y-axis 
    mpu6050_accel[ACCEL_Y_AXIS] = (int16_t)(
            mpu6050_accel_y_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_Y_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "ay = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Z-axis 
    mpu6050_accel[ACCEL_Z_AXIS] = (int16_t)(
            mpu6050_accel_z_calc(MPU6050_1_ADDRESS, mpu6050_accel[ACCEL_Z_AXIS]) * 
            NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "az = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

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
    
    uart_sendstring(USART2, "gx = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Y-axis 
    mpu6050_gyro[GYRO_Y_AXIS] = (int16_t)(mpu6050_gyro_y_calc(
                                              MPU6050_1_ADDRESS, 
                                              mpu6050_gyro[GYRO_Y_AXIS],
                                              mpu6050_gyro_offset[GYRO_Y_AXIS]) * 
                                                NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "gy = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Z-axis 
    mpu6050_gyro[GYRO_Z_AXIS] = (int16_t)(mpu6050_gyro_z_calc(
                                              MPU6050_1_ADDRESS, 
                                              mpu6050_gyro[GYRO_Z_AXIS],
                                              mpu6050_gyro_offset[GYRO_Z_AXIS]) * 
                                                NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "gz = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    //==============================================================

    // Go to a new line in the serial terminal 
    uart_send_new_line(USART2);

    // Delay 
    tim_delay_ms(TIM9, LOOP_DELAY);
}
