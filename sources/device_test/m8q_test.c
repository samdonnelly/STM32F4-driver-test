/**
 * @file m8q_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q driver and controller hardware test code implementation 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "m8q_test.h"
#include "m8q_config.h"
#include "stm32f4xx_it.h" 

//=======================================================================================


//=======================================================================================
// Notes 

// The ACK status function in the M8Q driver is not explicitly tested here as it's used 
// during initialization in tests 1 and 2. 

// The M8Q has no flash to store user settings. Instead they're saved in RAM which can 
// only be powered until the onboard backup battery loses power. For this reason, settings 
// must always be configured in setup. 

//=======================================================================================


//=======================================================================================
// Macros 

// Test 0 
#define M8Q_TEST_0_DATA_BUFF_LIM 400 
#define M8Q_TEST_0_READ_COUNT_LIM 90 
#define M8Q_TEST_0_OVERFLOW_COUNT_LO 40 
#define M8Q_TEST_0_OVERFLOW_COUNT_HI 70 

// Test 1 
#define M8Q_TEST_1_DATA_BUFF_LIM 0 
#define M8Q_TEST_1_COO_STR_LEN 25 
#define M8Q_TEST_1_READ_COUNT_LIM 60 
#define M8Q_TEST_1_LP_COUNT_LIM 90 
#define M8Q_TEST_1_NUM_PARAMS 12 

// Test 2 
#define M8Q_TEST_2_IDLE_COUNT 30 
#define M8Q_TEST_2_READ_COUNT 60 
#define M8Q_TEST_2_LP_EN_COUNT 90 
#define M8Q_TEST_2_LP_EX_COUNT 120 

//=======================================================================================


//=======================================================================================
// Enums 

/**
 * @brief M8Q test state number 
 */
typedef enum {
    M8Q_TEST_STATE_0, 
    M8Q_TEST_STATE_1, 
    M8Q_TEST_STATE_2 
} m8q_test_state_t; 

//=======================================================================================


//=======================================================================================
// Global variables 

// Test code data record 
typedef struct m8q_test_data_s 
{
    // Test 0 device data 
    uint8_t data_stream[M8Q_TEST_0_DATA_BUFF_LIM]; 

    // Test 1 device data 
    double latitude, longitude; 
    uint8_t lat_str[BYTE_11], lon_str[BYTE_12], utc_time[BYTE_10], utc_date[BYTE_7]; 
    uint8_t NS, EW, navstat_lock; 
    uint16_t navstat; 

    // Task scheduling 
    uint8_t schedule_counter; 
    uint8_t attempt_flag; 
}
m8q_test_data_t; 

static m8q_test_data_t test_data; 

//=======================================================================================


//=======================================================================================
// Prototypes 

/**
 * @brief Common/shared setup code 
 */
void m8q_test_general_init(void); 


/**
 * @brief Setup code for tests that use the driver config as well as LP and TX pins 
 */
void m8q_test_config_init(void); 


/**
 * @brief Common/shared test code 
 */
void m8q_test_general(void); 


/**
 * @brief Output driver data from test 1 
 * 
 * @param driver_status : driver function status 
 * @param output_state : test state that determines which output to use 
 */
void m8q_test_1_print(
    M8Q_STATUS driver_status, 
    m8q_test_state_t output_state); 


/**
 * @brief Output driver and controller data from test 2 
 */
void m8q_test_2_print(); 

//=======================================================================================


//=======================================================================================
// Setup code 

// Setup code for Test 0 
void m8q_test_0_init(void)
{
    m8q_test_general_init(); 

    // M8Q device setup 
    M8Q_STATUS init_check = m8q_init(
        I2C1, 
        m8q_config_no_pkt, 
        CLEAR, 
        CLEAR, 
        M8Q_TEST_0_DATA_BUFF_LIM); 
    
    // Check if there was a problem during device initialization. If so, output the fault 
    // to the serial terminal and halt to program. 
    if (init_check)
    {
        uart_sendstring(USART2, "\r\nDevice init status: "); 
        uart_send_integer(USART2, (int16_t)init_check); 

        while (TRUE); 
    }
}


// Setup code for Test 1 
void m8q_test_1_init(void)
{
    m8q_test_general_init(); 
    m8q_test_config_init(); 
}


// Setup code for Test 2 
void m8q_test_2_init(void)
{
    m8q_test_general_init(); 
    m8q_test_config_init(); 
    m8q_controller_init(TIM9); 
}


// Common/shared setup code 
void m8q_test_general_init(void)
{
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
        0x1388,  // ARR=5000, (5000 counts)*(100 us/count) = 500ms = 0.5s 
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
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 
    hd44780u_display_off(); 
    hd44780u_backlight_off(); 
#endif   // M8Q_TEST_SCREEN_ON_BUS 

    // Initialize variables 
    memset((void *)&test_data, CLEAR, sizeof(test_data)); 
}


// Setup code for tests that use the driver config as well as LP and TX pins 
void m8q_test_config_init(void)
{
    // M8Q device setup 
    M8Q_STATUS init_check = m8q_init(
        I2C1, 
        &m8q_config_pkt_0[0][0], 
        M8Q_CONFIG_NUM_MSG_PKT_0, 
        M8Q_CONFIG_MAX_LEN_PKT_0, 
        M8Q_TEST_1_DATA_BUFF_LIM); 

    // Set up low power and TX ready pins 
    M8Q_STATUS low_pwr_init_check = m8q_pwr_pin_init(GPIOC, PIN_10); 
    M8Q_STATUS txr_init_check = m8q_txr_pin_init(GPIOC, PIN_11); 

    // Check if there was a problem during device initialization. If so, output the faults 
    // to the serial terminal and halt to program. 
    if (init_check || low_pwr_init_check || txr_init_check)
    {
        uart_sendstring(USART2, "\r\nDevice init status: "); 
        uart_send_integer(USART2, (int16_t)init_check); 
        uart_sendstring(USART2, "\r\nLow power pin init status: "); 
        uart_send_integer(USART2, (int16_t)low_pwr_init_check); 
        uart_sendstring(USART2, "\r\nTX Ready pin init status: "); 
        uart_send_integer(USART2, (int16_t)txr_init_check); 

        while (TRUE); 
    }
}

//=======================================================================================


//=======================================================================================
// Test code 

// Test 0 code - default config, read raw data stream, don't use additional pins 
void m8q_test_0(void)
{
    // Local variables 
    M8Q_STATUS driver_status; 
    uint16_t data_size = CLEAR; 

    m8q_test_general(); 

    // Only interact with the device once per periodic interrupt. 
    if (test_data.attempt_flag)
    {
        test_data.attempt_flag = CLEAR_BIT; 
        
        // Periodically change between active and low power modes. 
        if (test_data.schedule_counter < M8Q_TEST_0_READ_COUNT_LIM)
        {
            // Within a certain range stop reading data to trigger a buffer overflow. 
            if ((test_data.schedule_counter < M8Q_TEST_0_OVERFLOW_COUNT_LO) || 
                (test_data.schedule_counter > M8Q_TEST_0_OVERFLOW_COUNT_HI))
            {
                driver_status = m8q_read_ds(test_data.data_stream, M8Q_TEST_0_DATA_BUFF_LIM); 

                switch (driver_status)
                {
                    case M8Q_OK: 
                        // Output the data stream 
                        uart_send_new_line(USART2); 
                        uart_sendstring(USART2, (char *)test_data.data_stream); 
                        break; 

                    case M8Q_NO_DATA_AVAILABLE: 
                        // Do nothing 
                        break; 

                    case M8Q_DATA_BUFF_OVERFLOW: 
                        // Indicate an overflow (data stream larger than max allowed buffer size) 
                        uart_sendstring(USART2, "\r\nBuffer overflow. Stream cleared.\r\n"); 
                        break; 

                    default:   // Everything else 
                        // Output the fault status 
                        uart_sendstring(USART2, "\r\nDriver fault: "); 
                        uart_send_integer(USART2, (int16_t)driver_status); 
                        uart_send_new_line(USART2); 
                        break; 
                }
            }
            else if (test_data.schedule_counter == M8Q_TEST_0_OVERFLOW_COUNT_LO)
            {
                uart_sendstring(USART2, "\r\nRead pause.\r\n"); 
            }
            else 
            {
                // Read the data size to keep the device I2C port active. 
                m8q_read_ds_size(&data_size); 
            }
        }
        else 
        {
            test_data.schedule_counter = CLEAR; 
        }
    }
}


// Test 1 code - device configured, driver data record messages used, additional pins used 
void m8q_test_1(void)
{
    // Local variables 
    M8Q_STATUS driver_status = M8Q_OK; 

    m8q_test_general(); 

    // Only interact with the device once per periodic interrupt. 
    if (test_data.attempt_flag)
    {
        test_data.attempt_flag = CLEAR_BIT; 

        // Periodically change between active and low power modes. 
        if (test_data.schedule_counter < M8Q_TEST_1_READ_COUNT_LIM)
        {
            // Only read and output data when there is data available 
            if (m8q_get_tx_ready())
            {
                driver_status = m8q_read_data(); 

                if (!driver_status)
                {
                    // Output the driver test data 
                    m8q_test_1_print(driver_status, M8Q_TEST_STATE_0); 
                }
                else 
                {
                    // Output the driver status 
                    m8q_test_1_print(driver_status, M8Q_TEST_STATE_1); 
                }
            }
        }
        else if (test_data.schedule_counter < M8Q_TEST_1_LP_COUNT_LIM)
        {
            m8q_set_low_pwr(); 

            // Output the low power status 
            m8q_test_1_print(driver_status, M8Q_TEST_STATE_2); 
        }
        else if (test_data.schedule_counter == M8Q_TEST_1_LP_COUNT_LIM)
        {
            m8q_clear_low_pwr(); 
        }
        else 
        {
            m8q_read_ds(test_data.data_stream, BYTE_1); 
            test_data.schedule_counter = CLEAR; 
        }
    }
}


// Test 2 code - device configured, device controller used 
void m8q_test_2(void)
{
    m8q_test_general(); 
    m8q_controller(); 

    // Only interact with the device once per periodic interrupt. 
    if (test_data.attempt_flag)
    {
        test_data.attempt_flag = CLEAR_BIT; 

        // If in the fault state then reset 
        if (m8q_get_state() == M8Q_FAULT_STATE)
        {
            m8q_set_reset_flag(); 
            test_data.schedule_counter = CLEAR; 
        }

        // Cycle between read, idle and low power states 
        switch (test_data.schedule_counter)
        {
            // If at time 1 then go to idle state 
            case M8Q_TEST_2_IDLE_COUNT: 
                m8q_set_idle_flag(); 
                break; 

            // If at time 2 then go back to read state 
            case M8Q_TEST_2_READ_COUNT: 
                m8q_set_read_flag(); 
                break; 

            // Go to the low power state 
            case M8Q_TEST_2_LP_EN_COUNT: 
                m8q_set_low_pwr_flag(); 
                break; 

            // Exit the low power state back to the read state 
            case M8Q_TEST_2_LP_EX_COUNT: 
                m8q_clear_low_pwr_flag(); 
                test_data.schedule_counter = CLEAR; 
                break; 

            default: 
                break; 
        }

        // Output the test information and data to the serial terminal 
        m8q_test_2_print(); 
    }
}


// Common/shared test code 
void m8q_test_general(void)
{
    if (handler_flags.tim1_up_tim10_glbl_flag)
    {
        handler_flags.tim1_up_tim10_glbl_flag = CLEAR; 
        test_data.schedule_counter++; 
        test_data.attempt_flag = SET_BIT; 
    }
}

//=======================================================================================


//=======================================================================================
// Test functions 

// Output the driver data from test 1 
void m8q_test_1_print(
    M8Q_STATUS driver_status, 
    m8q_test_state_t output_state)
{
    // Local variables 
    char latitude_str[M8Q_TEST_1_COO_STR_LEN], longitude_str[M8Q_TEST_1_COO_STR_LEN]; 
    GPIO_STATE lp_pin_state = GPIO_HIGH - gpio_read(GPIOC, (SET_BIT << PIN_10)); 
    int16_t lat_int, lon_int; 
    int32_t lat_frac, lon_frac; 

    switch (output_state)
    {
        case M8Q_TEST_STATE_0: 
            // Get all the driver data 
            test_data.latitude = m8q_get_position_lat(); 
            m8q_get_position_lat_str(test_data.lat_str, BYTE_11); 
            test_data.NS = m8q_get_position_NS(); 
            test_data.longitude = m8q_get_position_lon(); 
            m8q_get_position_lon_str(test_data.lon_str, BYTE_12); 
            test_data.EW = m8q_get_position_EW(); 
            test_data.navstat = m8q_get_position_navstat(); 
            test_data.navstat_lock = m8q_get_position_navstat_lock(); 
            m8q_get_time_utc_time(test_data.utc_time, BYTE_10); 
            m8q_get_time_utc_date(test_data.utc_date, BYTE_7); 

            // Format the latitude and longitude so it can be output 
            lat_int = (int16_t)test_data.latitude; 
            lat_frac = (int32_t)(SCALE_1E6*(test_data.latitude - (double)lat_int)); 
            if (lat_frac < 0) lat_frac = -lat_frac; 

            lon_int = (int16_t)test_data.longitude; 
            lon_frac = (int32_t)(SCALE_1E6*(test_data.longitude - (double)lon_int)); 
            if (lon_frac < 0) lon_frac = -lon_frac; 

            // Go to the top of the output block in the serial terminal 
            for (uint8_t i = CLEAR; i < M8Q_TEST_1_NUM_PARAMS; i++)
            {
                uart_sendstring(USART2, "\033[1A"); 
            }

            // Output all the data to the serial terminal for viewing 
            sprintf(latitude_str, "Latitude: %d.%ld", lat_int, lat_frac); 
            uart_sendstring(USART2, latitude_str); 
            uart_sendstring(USART2, "\r\nLatitude string: "); 
            uart_sendstring(USART2, (char *)test_data.lat_str); 
            uart_sendstring(USART2, "\r\nNS: "); 
            uart_send_integer(USART2, (int16_t)test_data.NS); 
            sprintf(longitude_str, "\r\nLongitude: %d.%ld", lon_int, lon_frac); 
            uart_sendstring(USART2, longitude_str); 
            uart_sendstring(USART2, "\r\nLongitude string: "); 
            uart_sendstring(USART2, (char *)test_data.lon_str); 
            uart_sendstring(USART2, "\r\nEW: "); 
            uart_send_integer(USART2, (int16_t)test_data.EW); 
            uart_sendstring(USART2, "\r\nNAVSTAT: "); 
            uart_send_integer(USART2, (int16_t)test_data.navstat); 
            uart_sendstring(USART2, "\r\nNAVSTAT lock: "); 
            uart_send_integer(USART2, (int16_t)test_data.navstat_lock); 
            uart_sendstring(USART2, "\r\nUTC time: "); 
            uart_sendstring(USART2, (char *)test_data.utc_time); 
            uart_sendstring(USART2, "\r\nUTC date: "); 
            uart_sendstring(USART2, (char *)test_data.utc_date); 
            uart_sendstring(USART2, "\r\nLP state: "); 
            uart_send_integer(USART2, (int16_t)lp_pin_state); 
            uart_sendstring(USART2, "\r\nDriver status: "); 
            uart_send_integer(USART2, (int16_t)driver_status); 
            uart_send_new_line(USART2); 

            break; 

        case M8Q_TEST_STATE_1: 
            uart_sendstring(USART2, "\033[1A"); 
            uart_sendstring(USART2, "\rDriver status: "); 
            uart_send_integer(USART2, (int16_t)driver_status); 
            uart_send_new_line(USART2); 

            break; 

        case M8Q_TEST_STATE_2: 
            uart_sendstring(USART2, "\033[1A\033[1A"); 
            uart_sendstring(USART2, "\rLP state: "); 
            uart_send_integer(USART2, (int16_t)lp_pin_state); 
            uart_sendstring(USART2, "\r\n\n"); 

            break; 

        default: 
            break; 
    }

}


// Output the driver and controller data from test 2 
void m8q_test_2_print(void)
{
    char latitude_str[M8Q_TEST_1_COO_STR_LEN], longitude_str[M8Q_TEST_1_COO_STR_LEN]; 
    int16_t lat_int, lon_int; 
    int32_t lat_frac, lon_frac; 

    // Get and format the coordinates 
    test_data.latitude = m8q_get_position_lat(); 
    test_data.longitude = m8q_get_position_lon(); 

    lat_int = (int16_t)test_data.latitude; 
    lat_frac = (int32_t)(SCALE_1E6*(test_data.latitude - (double)lat_int)); 
    if (lat_frac < 0) lat_frac = -lat_frac; 

    lon_int = (int16_t)test_data.longitude; 
    lon_frac = (int32_t)(SCALE_1E6*(test_data.longitude - (double)lon_int)); 
    if (lon_frac < 0) lon_frac = -lon_frac; 

    // Go to the top of the output block in the serial terminal 
    for (uint8_t i = CLEAR; i < 6; i++)
    {
        uart_sendstring(USART2, "\033[1A"); 
    }

    // Output all the data to the serial terminal for viewing 
    uart_sendstring(USART2, "\r\nState: "); 
    uart_send_integer(USART2, (int16_t)m8q_get_state()); 
    uart_sendstring(USART2, "\r\nLP flag: "); 
    uart_send_integer(USART2, (int16_t)m8q_get_lp_flag()); 
    sprintf(latitude_str, "\r\nLatitude: %d.%ld", lat_int, lat_frac); 
    uart_sendstring(USART2, latitude_str); 
    sprintf(longitude_str, "\r\nLongitude: %d.%ld", lon_int, lon_frac); 
    uart_sendstring(USART2, longitude_str); 
    uart_sendstring(USART2, "\r\nFault code = "); 
    uart_send_integer(USART2, (int16_t)m8q_get_fault_code()); 
    uart_send_new_line(USART2); 
}

//=======================================================================================
