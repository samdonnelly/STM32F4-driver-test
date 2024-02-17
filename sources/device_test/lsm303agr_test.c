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
#include "lsm303agr_config.h" 
#include "int_handlers.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define LSM303AGR_TEST_LPF_GAIN 0.2 
#define LSM303AGR_TEST_DISPLAY_COUNT 5 
#define LSM303AGR_TEST_MAX_STR_SIZE 60 

//=======================================================================================


//=======================================================================================
// Global variables 

// Test code data record 
typedef struct lsm303agr_test_data_s 
{
    // Magnetometer data 
    int16_t m_axis_data[NUM_AXES]; 
    int32_t m_field_data[NUM_AXES]; 
    int16_t m_heading; 

    // Status 
    LSM303AGR_STATUS driver_status; 

    // Task scheduling 
    uint8_t schedule_counter; 

    // Output 
    char output_str[LSM303AGR_TEST_MAX_STR_SIZE]; 
}
lsm303agr_test_data_t; 

static lsm303agr_test_data_t test_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

// Sets the offset data to be used during setup 
const int16_t* lsm303agr_test_offset_select(void); 


// Outputs the driver status and stops program execution 
void lasm303agr_test_fault_state(void); 

//=======================================================================================


//=======================================================================================
// Setup code

void lsm303agr_test_init(void)
{
    // Initialize variables 
    const int16_t *offsets = lsm303agr_test_offset_select(); 
    memset((void *)test_data.m_axis_data, CLEAR, sizeof(test_data.m_axis_data)); 
    memset((void *)test_data.m_field_data, CLEAR, sizeof(test_data.m_field_data)); 
    test_data.m_heading = CLEAR; 
    test_data.driver_status = LSM303AGR_OK; 
    test_data.schedule_counter = CLEAR; 
    memset((void *)test_data.output_str, CLEAR, sizeof(test_data.output_str)); 

    // Initialize GPIO ports 
    gpio_port_init(); 

    // General purpose timer 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Periodic (counter update) interrupt timer (for event timing) 
    tim_9_to_11_counter_init(
        TIM10, 
        TIM_84MHZ_100US_PSC, 
        0x03E8,  // ARR=1000, (1000 counts)*(100us/count) = 100ms = 0.1s 
        TIM_UP_INT_ENABLE); 
    tim_enable(TIM10); 

    // Initialize UART (serial terminal output) 
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

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

    // Screen initialization 
#if M8Q_TEST_SCREEN_ON_BUS 
    // If the HD44780U screen is on the same I2C bus as the LSM303AGR then the screen 
    // must be set up first to prevent it from interfering with the bus. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_display_off(); 
    hd44780u_backlight_off(); 
#endif   // M8Q_TEST_SCREEN_ON_BUS 

    // LSM303AGR driver init 
    test_data.driver_status = lsm303agr_m_init(
        I2C1, 
        offsets, 
        LSM303AGR_TEST_LPF_GAIN, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 

    if (test_data.driver_status)
    {
        lasm303agr_test_fault_state(); 
    }

    // Set the initial serial terminal message 
#if LSM303AGR_TEST_AXIS 
    uart_sendstring(USART2, "Axis data [x,y,z] (digital output, mgauss):"); 
#elif LSM303AGR_TEST_HEADING 
    uart_sendstring(USART2, "Heading (deg*10):"); 
#endif 
    uart_send_new_line(USART2); 
} 

//=======================================================================================


//=======================================================================================
// Test code 

void lsm303agr_test_app(void)
{
    // Test code for the LSM303AGR here 

    // Periodically update and display data 
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 
        test_data.schedule_counter++; 
        
        // Update the magnetometer data 
        test_data.driver_status = lsm303agr_m_update(); 

#if LSM303AGR_TEST_AXIS 

        // Display the heading (every x counts) 
        if (test_data.schedule_counter >= LSM303AGR_TEST_DISPLAY_COUNT)
        {
            // Update and get the latest axis data 
            lsm303agr_m_get_axis_data(test_data.m_axis_data); 
            lsm303agr_m_get_field(test_data.m_field_data); 

            test_data.schedule_counter = CLEAR; 
            snprintf(
                test_data.output_str, 
                LSM303AGR_TEST_MAX_STR_SIZE, 
                "%d, %ld     \r\n%d, %ld     \r\n%d, %ld     \r\n", 
                test_data.m_axis_data[X_AXIS], 
                test_data.m_field_data[X_AXIS], 
                test_data.m_axis_data[Y_AXIS], 
                test_data.m_field_data[Y_AXIS], 
                test_data.m_axis_data[Z_AXIS], 
                test_data.m_field_data[Z_AXIS]); 
            uart_sendstring(USART2, test_data.output_str); 
            uart_sendstring(USART2, "\033[1A\033[1A\033[1A"); 
        }

#elif LSM303AGR_TEST_HEADING 

        // Update and get the latest heading 
        test_data.m_heading = lsm303agr_m_get_heading(); 

        // Display the heading (every x counts) 
        if (test_data.schedule_counter >= LSM303AGR_TEST_DISPLAY_COUNT)
        {
            test_data.schedule_counter = CLEAR; 
            uart_sendstring(USART2, "\r"); 
            uart_send_integer(USART2, test_data.m_heading); 
            uart_send_spaces(USART2, UART_SPACE_3); 
        }

#endif   // LSM303AGR_TEST_HEADING 

        // Check status 
        if (test_data.driver_status)
        {
            uart_send_new_line(USART2); 
            lasm303agr_test_fault_state(); 
        }
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Sets the offset data to be used during setup 
const int16_t* lsm303agr_test_offset_select(void)
{
#if (LSM303AGR_TEST_HEADING && LSM303AGR_TEST_CALIBRATION) || LSM303AGR_TEST_AXIS 
        return lsm303agr_calibrate_offsets; 
#else 
        return lsm303agr_config_dir_offsets_1; 
#endif 
}


// Outputs the driver status and stops program execution 
void lasm303agr_test_fault_state(void)
{
    uart_sendstring(USART2, "\r\nMagnetometer init status: "); 
    uart_send_integer(USART2, (int16_t)test_data.driver_status); 
    tim_disable(TIM10); 
    while (TRUE); 
}

//=======================================================================================
