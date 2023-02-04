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


//=======================================================================================
// Function prototypes 
//=======================================================================================


//=======================================================================================
// Global variables 
//=======================================================================================


//=======================================================================================
// Test code 

// Setup code
void mpu6050_test_init()
{
    // Local variables 
    uint8_t mpu6050_init_status;    // Store the driver init status 
    uint8_t mpu_self_test_result;   // Store the self-test status 

    //===================================================
    // Peripherals 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART2
    uart_init(USART2, UART_BAUD_9600, UART_CLOCK_42);

    // Initialize I2C1
    i2c1_init(
        I2C1, 
        I2C1_SDA_PB9,
        I2C1_SCL_PB8,
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42);
    
    //===================================================

    //===================================================
    // Accelerometer initialization 
    
    // Initialize the accelerometer 
    // The return value can be used to enter an error state
    mpu6050_init_status = mpu6050_init(I2C1, 
                                       MPU6050_1_ADDRESS,
                                       DLPF_CFG_1,
                                       SMPLRT_DIV_0,
                                       AFS_SEL_4,
                                       FS_SEL_500);

    // Return the status of the accelerometer WHO_AM_I register 
    switch(mpu6050_init_status)
    {
        case TRUE:
            uart_sendstring(USART2, "Device seen\r\n");
            break;
        
        case FALSE:
            uart_sendstring(USART2, "Device not seen\r\n");
            break;

        default:
            break;
    }

    // MPU6050 self-test 
    mpu_self_test_result = mpu6050_self_test();
    uart_sendstring(USART2, "MPU6050 Self-Test Result = ");
    uart_send_integer(USART2, (int16_t)(mpu_self_test_result));
    uart_send_new_line(USART2);

    //===================================================

    //===================================================
    // Setup 

#if MPU6050_CONTROLLER_TEST 

#else   // MPU6050_CONTROLLER_TEST 

#endif   // MPU6050_CONTROLLER_TEST 

    //===================================================
} 


// Test code 
void mpu6050_test_app()
{
#if MPU6050_CONTROLLER_TEST 

#else   // MPU6050_CONTROLLER_TEST

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
            mpu6050_accel_offset,
            mpu6050_gyro_offset);
        
        one_time++;
    }

    //==================================================
    // Read data 
    //==================================================

    //==================================================
    // Get data 
    //==================================================

    //==================================================
    // Display data 
    //==================================================

    //==============================================================
    // Temperature 

    // mpu6050_temp_sensor = mpu6050_temp_read();
    mpu6050_temp_read();
    mpu6050_temp_sensor = (int16_t)(mpu6050_get_temp(mpu6050_temp_sensor) * 
                                    NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "temp = ");
    uart_send_integer(USART2, mpu6050_temp_sensor);
    uart_send_spaces(USART2, UART2_2_SPACES);

    //==============================================================


    //==============================================================
    // Accelerometer 

    mpu6050_accel_read(mpu6050_accel);

    // X-axis
    mpu6050_accel[ACCEL_X_AXIS] = (int16_t)(
            mpu6050_get_accel_x(mpu6050_accel[ACCEL_X_AXIS]) * NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "ax = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Y-axis 
    mpu6050_accel[ACCEL_Y_AXIS] = (int16_t)(
            mpu6050_get_accel_y(mpu6050_accel[ACCEL_Y_AXIS]) * NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "ay = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Z-axis 
    mpu6050_accel[ACCEL_Z_AXIS] = (int16_t)(
            mpu6050_get_accel_z(mpu6050_accel[ACCEL_Z_AXIS]) * NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "az = ");
    uart_send_integer(USART2, mpu6050_accel[ACCEL_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    //==============================================================


    //==============================================================
    // Gyroscope 

    mpu6050_gyro_read(mpu6050_gyro);

    // X-axis
    mpu6050_gyro[GYRO_X_AXIS] = (int16_t)(mpu6050_get_gyro_x(
                                              mpu6050_gyro[GYRO_X_AXIS],
                                              mpu6050_gyro_offset[GYRO_X_AXIS]) * 
                                              NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "gx = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_X_AXIS]); 
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Y-axis 
    mpu6050_gyro[GYRO_Y_AXIS] = (int16_t)(mpu6050_get_gyro_y(
                                              mpu6050_gyro[GYRO_Y_AXIS],
                                              mpu6050_gyro_offset[GYRO_Y_AXIS]) * 
                                              NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "gy = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Y_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    // Z-axis 
    mpu6050_gyro[GYRO_Z_AXIS] = (int16_t)(mpu6050_get_gyro_z(
                                              mpu6050_gyro[GYRO_Z_AXIS],
                                              mpu6050_gyro_offset[GYRO_Z_AXIS]) * 
                                              NO_DECIMAL_SCALAR);
    
    uart_sendstring(USART2, "gz = ");
    uart_send_integer(USART2, mpu6050_gyro[GYRO_Z_AXIS]);
    uart_send_spaces(USART2, UART2_2_SPACES);

    //==============================================================

    // Delay 
    tim_delay_ms(TIM9, LOOP_DELAY);

    // Go to a new line in the serial terminal 
    // uart_send_new_line(USART2);
    uart_sendstring(USART2, "\r"); 

#endif   // MPU6050_CONTROLLER_TEST 
}

//=======================================================================================


//=======================================================================================
// Test functions 
//=======================================================================================
