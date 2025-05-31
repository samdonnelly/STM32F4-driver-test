/**
 * @file lsm303agr_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief LSM303AGR test code 
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

    // Initialize GPIO ports 
    gpio_port_init(); 

    // Periodic (counter update) interrupt timer (for event timing) 
    tim_9_to_11_counter_init(
        test_data.tim, 
        TIM_84MHZ_100US_PSC, 
        LSM303AGR_TEST_INT_COUNTER, 
        TIM_UP_INT_ENABLE); 
    tim_enable(test_data.tim); 

    // Initialize UART (serial terminal output) 
    uart_init(
        test_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR_BIT,             // STOP bits 
        UART_FRAC_42_115200,   // MotionCal requires this baudrate 
        UART_MANT_42_115200, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE); 

    // Initialize I2C (to communicate with device) 
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
    uart_send_str(test_data.uart, "Axis (milligauss), calibrated axis (milligauss), heading (deg*10)\r\n"); 
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
