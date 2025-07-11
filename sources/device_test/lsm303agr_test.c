/**
 * @file lsm303agr_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR test code 
 * 
 * @details Setup 
 *          - Hardware 
 *            * STM32F4 microcontroller with a serial connection to a PC. 
 *            * An LSM303AGR IMU connected to the STM32F4 
 *          - Software 
 *            * Serial monitor on a PC to allow the exchange of info with the STM32F4. 
 *            * MotionCal to obtain calibration values (but not used directly in this 
 *              test). More on MotionCal below. 
 *          
 *          Configuration 
 *          - TIM 
 *            * A timer is configured to create a periodic interrupt which controls when 
 *              to read and output device data. 
 *          - UART 
 *            * UART is configured to provide a serial terminal output, for either 
 *              displaying data and providing the data to MotionCal. MotionCal requires 
 *              115200 baud rate so that is what is configured. 
 *          - I2C 
 *            * I2C is configured to communicate with the LSM303AGR device. 
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
 *          - The test will initialize all peripherals then configure the LSM303AGR 
 *            device. Setters to correct magnetometer axis data output are called and 
 *            the user must define the values provided to these functions. See the 
 *            comments on these functions in the test code and the function descriptions 
 *            for more details. When the code is running, driver data such as axis data 
 *            and heading will periodically be output to the serial terminal for the user 
 *            to see. LSM303AGR_TEST_DISPLAY_COUNT can be used to adjust the output 
 *            rate of the data. 
 *          
 *          - This test supports data output for MotionCal which is a software used to 
 *            find hard-iron offsets and soft-iron scaling values to correct the 
 *            magnetoeter axis outputs and obtain a more accurate magnetic heading. 
 *            LSM303AGR_TEST_CALIBRATION can be set in the hardware config to change 
 *            the data output to a format supported by MotionCal. For details on how to 
 *            use MotionCal see the following tutorial: 
 *            - https://www.digikey.ca/en/maker/projects/how-to-calibrate-a-magnetometer/50f6bc8f36454a03b664dca30cf33a8b 
 *            For details on how to properly apply the values obtained from MotionCal to 
 *            the LSM303AGR driver, see the driver calibration setter function description. 
 * 
 * @version 0.1
 * @date 2023-06-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "lsm303agr_test.h" 
#include "device_config.h" 
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Configuration 
#define LSM303AGR_TEST_MAX_STR_SIZE 160     // Max output string size 
#define LSM303AGR_TEST_INT_COUNTER 0x03E8   // ARR=1000, (1000 counts)*(100us/count) = 100ms = 0.1s 

//=======================================================================================


//=======================================================================================
// Global variables 

// Test code data record 
typedef struct lsm303agr_test_data_s 
{
    // Peripherals 
    USART_TypeDef *uart; 
    TIM_TypeDef *tim; 

    // Magnetometer data 
    int16_t m_axis[NUM_AXES]; 
    int16_t m_axis_cal[NUM_AXES]; 
    int16_t m_heading; 

    // Status 
    LSM303AGR_STATUS driver_status; 

    // Task scheduling 
    uint8_t schedule_counter; 

    // Output 
    char output_str[LSM303AGR_TEST_MAX_STR_SIZE]; 
}
lsm303agr_test_data_t; 

// Data record instance 
static lsm303agr_test_data_t test_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Outputs the driver status and stops program execution 
 */
void lasm303agr_test_fault_state(void); 

//=======================================================================================


//=======================================================================================
// Setup code

void lsm303agr_test_init(void)
{
    // Initialize variables 
    test_data.uart = USART2; 
    test_data.tim = TIM10; 
    memset((void *)test_data.m_axis, CLEAR, sizeof(test_data.m_axis)); 
    memset((void *)test_data.m_axis_cal, CLEAR, sizeof(test_data.m_axis_cal)); 
    test_data.m_heading = CLEAR; 
    test_data.driver_status = LSM303AGR_OK; 
    test_data.schedule_counter = CLEAR; 
    memset((void *)test_data.output_str, CLEAR, sizeof(test_data.output_str)); 

    // GPIO 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer 
    tim_9_to_11_counter_init(
        test_data.tim, 
        TIM_84MHZ_100US_PSC, 
        LSM303AGR_TEST_INT_COUNTER, 
        TIM_UP_INT_ENABLE); 
    tim_enable(test_data.tim); 

    // UART - serial terminal output 
    uart_init(
        test_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE, 
        CLEAR_BIT, 
        UART_FRAC_42_115200, 
        UART_MANT_42_115200, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // I2C - LSM303AGR 
    i2c_init(
        I2C1, 
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

    // LSM303AGR driver init 
    test_data.driver_status |= lsm303agr_m_init(
        I2C1, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 

    // LSM303AGR hard-iron offset register set. If this setting is not desired then set 
    // 'lsm303agr_hi_offset_reg' values to zero. If this is not desired in a practical 
    // application then this function can simply not be called. If using this setting 
    // then it's recommended not to use the calibration value setter below. 
    test_data.driver_status |= lsm303agr_m_offset_reg_set(lsm303agr_hi_offset_reg); 

    // LSM303AGR hard and soft-iron calibraton value set. If this setting is not desired 
    // then set 'lsm303agr_hi_offset' and 'lsm303agr_sio_values' values to zero, and 
    // 'lsm303agr_sid_values' values to 1 (see setter description for more details). If 
    // this setting is not desdired in a practical application then this function can 
    // simply not be called. If using this setting then it's recommended not to use the 
    // hard-iron offset register setter above. 
    lsm303agr_m_calibration_set(lsm303agr_hi_offset, lsm303agr_sid_values, lsm303agr_sio_values); 

    if (test_data.driver_status)
    {
        lasm303agr_test_fault_state(); 
    }

    // Set the initial serial terminal message 
#if LSM303AGR_TEST_CALIBRATION
    uart_send_str(test_data.uart, "Raw axis data (milligauss)\r\n"); 
#else 
    uart_send_str(test_data.uart, 
                  "Axis (milligauss), calibrated axis (milligauss), heading (deg*10)\r\n"); 
#endif   // LSM303AGR_TEST_CALIBRATION 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void lsm303agr_test_app(void)
{
    // Periodically update and display data 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 
        
        // Update the magnetometer data and check the driver status 
        test_data.driver_status = lsm303agr_m_update(); 

        if (test_data.driver_status != LSM303AGR_OK)
        {
            uart_send_new_line(test_data.uart); 
            lasm303agr_test_fault_state(); 
        }

        // Get and display the magnetometer data 
        lsm303agr_m_get_axis(test_data.m_axis); 
        lsm303agr_m_get_calibrated_axis(test_data.m_axis_cal); 
        test_data.m_heading = lsm303agr_m_get_heading(); 

        if (++test_data.schedule_counter >= LSM303AGR_TEST_DISPLAY_COUNT)
        {
            test_data.schedule_counter = CLEAR; 

#if LSM303AGR_TEST_CALIBRATION 

            // A software called MotionCal is used calibrate the magnetometer axis data. 
            // When in calibration mode, the data output is formatted such that MotionCal 
            // will read it. This output has space for accelerometer and gyroscope data 
            // as well. 

            snprintf(
                test_data.output_str, 
                LSM303AGR_TEST_MAX_STR_SIZE, 
                "Raw:0,0,0,0,0,0,%d,%d,%d\r\n", 
                test_data.m_axis[X_AXIS], 
                test_data.m_axis[Y_AXIS], 
                test_data.m_axis[Z_AXIS]); 
            uart_send_str(test_data.uart, test_data.output_str); 

#else 
            
            // In normal mode, all the data read from the magnetometer is displayed for 
            // the user to see. 

            snprintf(
                test_data.output_str, 
                LSM303AGR_TEST_MAX_STR_SIZE, 
                "\r" \
                "x_axis: %d     \r\n" \
                "y_axis: %d     \r\n" \
                "z_axis: %d     \r\n" \
                "x_axis_cal: %d     \r\n" \
                "y_axis_cal: %d     \r\n" \
                "z_axis_cal: %d     \r\n" \
                "heading: %d     ", 
                test_data.m_axis[X_AXIS], 
                test_data.m_axis[Y_AXIS], 
                test_data.m_axis[Z_AXIS], 
                test_data.m_axis_cal[X_AXIS], 
                test_data.m_axis_cal[Y_AXIS], 
                test_data.m_axis_cal[Z_AXIS], 
                test_data.m_heading); 
            uart_send_str(test_data.uart, test_data.output_str); 
            uart_cursor_move(test_data.uart, UART_CURSOR_UP, 6); 

#endif   // LSM303AGR_TEST_CALIBRATION 
        }
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Outputs the driver status and stops program execution 
void lasm303agr_test_fault_state(void)
{
    uart_send_str(test_data.uart, "\r\nLSM303AGR status: "); 
    uart_send_integer(test_data.uart, (int16_t)test_data.driver_status); 
    tim_disable(test_data.tim); 
    while (TRUE); 
}

//=======================================================================================
