/**
 * @file mpu6050_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief MPU-6050 test code 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *            * 1 or 2 MPU-6050 IMUs connected to the STM32F4. 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *          
 *          Configuration 
 *          - TIM 
 *            * A timer is configured to create a periodic interrupt which controls when 
 *              to read and output device data. 
 *          - UART 
 *            * UART is configured to provide a serial terminal output both for device 
 *              data and driver status faults. 
 *          - I2C 
 *            * I2C is configured to communicate with the MPU-6050 device. 
 *          - Interrupts 
 *            * An interrupt is configured for the timer to create a periodic interrupt 
 *              to control when to read and output device data. 
 *          
 *          Dependencies 
 *          - STM32F4 driver library 
 *            * This library provides an interface to the device and the peripherals 
 *              used in the test. 
 *          
 *          Procedure 
 *          - During setup, the IMU will run a self test to check its axes. If this fails 
 *            then the status will be displayed and the code will be haulted. If successful 
 *            then the code proceeds to read and display raw and formatted IMU data 
 *            periodically in the serial terminal. If a fault occurs at any time then the 
 *            test will be haulted and the driver status displayed. 
 *          - A second device can be enabled that does the same thing in parallel to the 
 *            the first device. 
 *            
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
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Data 
#define MPU6050_DEV1_STBY_MASK 0x00         // Device 1 axis standby status mask 
#define MPU6050_DEV2_STBY_MASK 0x00         // Device 2 axis standby status mask 
#define MPU6050_SMPLRT_DIV 0                // Sample Rate Divider 

// Timing 
#define MPU6050_DRIVER_ST_DELAY 10          // Delay (blocking) after self test (ms) 
#define MPU6050_TEST_INT_COUNTER 0x07D0     // ARR=2000, (2000 counts)*(100us/count) = 200ms = 0.2s 

// Data output 
#define MPU6050_TEST_MAX_STR_SIZE 150       // Max output string size 
#define MPU6050_TEST_OUTPUT_LINES 2         // Number of output lines per IMU 

//=======================================================================================


//=======================================================================================
// Global data 

typedef struct mpu6050_test_imu_data_s
{
    int16_t temp_raw, accel_raw[NUM_AXES], gyro_raw[NUM_AXES]; 
    float temp, accel[NUM_AXES], gyro[NUM_AXES]; 
    uint8_t st_result; 
    MPU6050_STATUS status; 
}
mpu6050_test_imu_data_t; 


typedef struct mpu6050_test_data_s 
{
    USART_TypeDef *uart; 
    I2C_TypeDef *i2c; 
    TIM_TypeDef *tim_periodic; 

    // Data output 
    char output_raw[MPU6050_TEST_MAX_STR_SIZE]; 
    char output_formatted[MPU6050_TEST_MAX_STR_SIZE]; 
    uint8_t cursor_lines; 

    mpu6050_test_imu_data_t imu1; 

#if MPU6050_SECOND_DEVICE 

    mpu6050_test_imu_data_t imu2; 

#endif   // MPU6050_SECOND_DEVICE 
}
mpu6050_test_data_t; 

static mpu6050_test_data_t mpu6050_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Read, format and output IMU data for a given device number 
 * 
 * @param device_num : IMU number 
 * @param imu_data : IMU data to use 
 */
void mpu6050_test_read_format_output(
    device_number_t device_num, 
    mpu6050_test_imu_data_t imu_data); 


/**
 * @brief Output the driver status and stop program execution 
 */
void mpu6050_test_fault_state(void); 

//=======================================================================================


//=======================================================================================
// Setup code 

void mpu6050_test_init()
{
    mpu6050_data.uart = USART2; 
    mpu6050_data.i2c = I2C1; 
    mpu6050_data.tim_periodic = TIM10; 
    memset((void *)mpu6050_data.output_raw, CLEAR, sizeof(mpu6050_data.output_raw)); 
    memset((void *)mpu6050_data.output_formatted, CLEAR, sizeof(mpu6050_data.output_formatted)); 
    mpu6050_data.cursor_lines = MPU6050_TEST_OUTPUT_LINES + MPU6050_TEST_OUTPUT_LINES*MPU6050_SECOND_DEVICE; 

    mpu6050_data.imu1.temp_raw = CLEAR; 
    memset((void *)mpu6050_data.imu1.accel_raw, CLEAR, sizeof(mpu6050_data.imu1.accel_raw)); 
    memset((void *)mpu6050_data.imu1.gyro_raw, CLEAR, sizeof(mpu6050_data.imu1.gyro_raw)); 
    mpu6050_data.imu1.temp = CLEAR; 
    memset((void *)mpu6050_data.imu1.accel, CLEAR, sizeof(mpu6050_data.imu1.accel)); 
    memset((void *)mpu6050_data.imu1.gyro, CLEAR, sizeof(mpu6050_data.imu1.gyro)); 
    mpu6050_data.imu1.st_result = CLEAR; 
    mpu6050_data.imu1.status = MPU6050_OK; 

#if MPU6050_SECOND_DEVICE 

    mpu6050_data.imu2.temp_raw = CLEAR; 
    memset((void *)mpu6050_data.imu2.accel_raw, CLEAR, sizeof(mpu6050_data.imu2.accel_raw)); 
    memset((void *)mpu6050_data.imu2.gyro_raw, CLEAR, sizeof(mpu6050_data.imu2.gyro_raw)); 
    mpu6050_data.imu2.temp = CLEAR; 
    memset((void *)mpu6050_data.imu2.accel, CLEAR, sizeof(mpu6050_data.imu2.accel)); 
    memset((void *)mpu6050_data.imu2.gyro, CLEAR, sizeof(mpu6050_data.imu2.gyro)); 
    mpu6050_data.imu2.st_result = CLEAR; 
    mpu6050_data.imu2.status = MPU6050_OK; 

#endif   // MPU6050_SECOND_DEVICE 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        mpu6050_data.tim_periodic, 
        TIM_84MHZ_100US_PSC, 
        MPU6050_TEST_INT_COUNTER, 
        TIM_UP_INT_ENABLE); 
    tim_enable(mpu6050_data.tim_periodic); 

    // UART - serial terminal output 
    uart_init(
        mpu6050_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        CLEAR_BIT, 
        UART_FRAC_42_115200, 
        UART_MANT_42_115200, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // Initialize I2C1
    i2c_init(
        mpu6050_data.i2c, 
        PIN_9, 
        GPIOB, 
        PIN_8, 
        GPIOB, 
        I2C_MODE_SM,
        I2C_APB1_42MHZ,
        I2C_CCR_SM_42_100,
        I2C_TRISE_1000_42); 

    // Initialize interrupt handler flags and enable the periodic timer interrupt handler 
    int_handler_init(); 
    nvic_config(TIM1_UP_TIM10_IRQn, EXTI_PRIORITY_0); 

    //===================================================
    // MPU-6050 initialization and setup 
    
    // Initialize the accelerometer 
    mpu6050_data.imu1.status |= mpu6050_init(
        DEVICE_ONE, 
        mpu6050_data.i2c, 
        MPU6050_ADDR_1,
        MPU6050_DEV1_STBY_MASK, 
        MPU6050_DLPF_CFG_1,
        MPU6050_SMPLRT_DIV,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);

#if MPU6050_INT_PIN 

    // Set up the INT pin 
    mpu6050_int_pin_init(GPIOC, PIN_11); 

#endif   // MPU6050_INT_PIN 

    // MPU6050 self-test 
    mpu6050_data.imu1.status |= mpu6050_self_test(DEVICE_ONE, &mpu6050_data.imu1.st_result); 

    if (mpu6050_data.imu1.status != MPU6050_OK)
    {
        mpu6050_test_fault_state(); 
    }

#if MPU6050_SECOND_DEVICE 

    // Initialize the second accelerometer 
    mpu6050_data.imu2.status |= mpu6050_init(
        DEVICE_TWO, 
        mpu6050_data.i2c, 
        MPU6050_ADDR_2,
        MPU6050_DEV2_STBY_MASK, 
        MPU6050_DLPF_CFG_1,
        MPU6050_SMPLRT_DIV,
        MPU6050_AFS_SEL_4,
        MPU6050_FS_SEL_500);

    // MPU6050 self-test 
    mpu6050_data.imu2.status |= mpu6050_self_test(DEVICE_TWO, &mpu6050_data.imu2.st_result); 

    if (mpu6050_data.imu2.status != MPU6050_OK)
    {
        mpu6050_test_fault_state(); 
    }

#endif   // MPU6050_SECOND_DEVICE 

    //===================================================
} 

//=======================================================================================


//=======================================================================================
// Test code 

void mpu6050_test_app()
{
    // Periodically update and display data 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 

        // Reset the cursor position to overwrite the old data 
        uart_cursor_move(mpu6050_data.uart, UART_CURSOR_UP, mpu6050_data.cursor_lines); 
        uart_send_str(mpu6050_data.uart, "\r"); 

        mpu6050_test_read_format_output(DEVICE_ONE, mpu6050_data.imu1); 

#if MPU6050_SECOND_DEVICE 

        mpu6050_test_read_format_output(DEVICE_TWO, mpu6050_data.imu2); 

#endif   // MPU6050_SECOND_DEVICE 
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Read, format and output IMU data for a given device number 
void mpu6050_test_read_format_output(
    device_number_t device_num, 
    mpu6050_test_imu_data_t imu_data)
{
    // Update the accelerometer, temperature and gyroscope readings for device one 
    imu_data.status = mpu6050_update(device_num); 

    if (imu_data.status != MPU6050_OK)
    {
        mpu6050_test_fault_state(); 
    }

    // Get the raw temperature, accelerometer and gyroscope readings 
    imu_data.temp_raw = mpu6050_get_temp_raw(device_num); 
    mpu6050_get_accel_axis(device_num, imu_data.accel_raw); 
    mpu6050_get_gyro_axis(device_num, imu_data.gyro_raw); 

    // Get the formatted temp (degC), accelerometer (g's) and gyroscope (deg/s) data 
    imu_data.temp = mpu6050_get_temp(device_num); 
    mpu6050_get_accel_axis_gs(device_num, imu_data.accel); 
    mpu6050_get_gyro_axis_rate(device_num, imu_data.gyro); 

    // Format the raw data into a string 
    snprintf(mpu6050_data.output_raw, 
             MPU6050_TEST_MAX_STR_SIZE, 
             "temp_r = %d ax_r = %d ay_r = %d az_r = %d gx_r = %d gy_r = %d gz_r = %d      \r\n", 
             imu_data.temp_raw, 
             imu_data.accel_raw[X_AXIS], 
             imu_data.accel_raw[Y_AXIS], 
             imu_data.accel_raw[Z_AXIS], 
             imu_data.gyro_raw[X_AXIS], 
             imu_data.gyro_raw[Y_AXIS], 
             imu_data.gyro_raw[Z_AXIS]); 

    // Format the formatted data into a striing 
    snprintf(mpu6050_data.output_formatted, 
             MPU6050_TEST_MAX_STR_SIZE, 
             "temp_f = %d ax_f = %d ay_f = %d az_f = %d gx_f = %d gy_f = %d gz_f = %d      \r\n", 
             (int16_t)(imu_data.temp * SCALE_100), 
             (int16_t)(imu_data.accel[X_AXIS] * SCALE_100), 
             (int16_t)(imu_data.accel[Y_AXIS] * SCALE_100), 
             (int16_t)(imu_data.accel[Z_AXIS] * SCALE_100), 
             (int16_t)(imu_data.gyro[X_AXIS] * SCALE_100), 
             (int16_t)(imu_data.gyro[Y_AXIS] * SCALE_100), 
             (int16_t)(imu_data.gyro[Z_AXIS] * SCALE_100)); 

    // Display the data in the serial terminal 
    uart_send_str(mpu6050_data.uart, mpu6050_data.output_raw); 
    uart_send_str(mpu6050_data.uart, mpu6050_data.output_formatted); 
}


// Output the driver status and stop program execution 
void mpu6050_test_fault_state(void)
{
    tim_disable(mpu6050_data.tim_periodic); 

    snprintf(mpu6050_data.output_formatted, 
             MPU6050_TEST_MAX_STR_SIZE, 
             "\r\nMPU-6050 1\r\nStatus: %lu\r\nSelf-Test: %u", 
             mpu6050_data.imu1.status, 
             mpu6050_data.imu1.st_result); 
    uart_send_str(mpu6050_data.uart, mpu6050_data.output_formatted); 

#if MPU6050_SECOND_DEVICE 

    snprintf(mpu6050_data.output_formatted, 
             MPU6050_TEST_MAX_STR_SIZE, 
             "\r\nMPU-6050 2\r\nStatus: %lu\r\nSelf-Test: %u", 
             mpu6050_data.imu2.status, 
             mpu6050_data.imu2.st_result); 
    uart_send_str(mpu6050_data.uart, mpu6050_data.output_formatted); 

#endif   // MPU6050_SECOND_DEVICE 

    while (TRUE); 
}

//=======================================================================================
