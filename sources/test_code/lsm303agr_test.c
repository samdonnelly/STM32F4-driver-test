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

//=======================================================================================


//=======================================================================================
// TODO 
// - Test magnetometer heading with low power, idle (mode), different ODR, LFP enabled, 
//   and offset cancellation 
// - Change double calculations to scaled integer calculations (int32_t) 
//=======================================================================================


//=======================================================================================
// Global variables 

static int16_t offsets[8]; 

#if LSM303AGR_TEST_AXIS 
static int16_t mx_data; 
static int16_t my_data; 
static int16_t mz_data; 
#endif   // LSM303AGR_TEST_AXIS 

#if LSM303AGR_TEST_NAV 
#endif   // LSM303AGR_TEST_NAV 

//=======================================================================================


//=======================================================================================
// Function prototypes 

#if LSM303AGR_TEST_NAV 

/**
 * @brief GPS coordinate surface distance 
 * 
 * @details 
 * 
 * @param gps1 
 * @param gps2 
 * @param surf_dist_thresh 
 * @return uint8_t 
 */
uint8_t lsm303agr_gps_rad(
    double *gps1, 
    double *gps2, 
    double surf_dist_thresh); 


/**
 * @brief Heading error - done every heading update (~10Hz) 
 * 
 * @details 
 * 
 * @param heading_desired 
 * @param heading_current 
 * @return int16_t 
 */
int16_t lsm303agr_heading_error(
    int16_t heading_desired, 
    int16_t heading_current); 

#endif   // LSM303AGR_TEST_NAV 

//=======================================================================================


// Setup code
void lsm303agr_test_init(void)
{
    // Setup code for the gpio_test here 

    //===================================================
    // Initialize peripherals 

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
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_BAUD_9600, 
        UART_CLOCK_42); 

    // Initialize I2C1
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
    
    //===================================================

#if LSM303AGR_TEST_NAV 

    //===================================================
    // Navigation test init 

    // Screen init 

    // GPS init 

    // PWM init 

    //===================================================

#endif   // LSM303AGR_TEST_NAV 

    //==================================================
    // LSM303AGR init 

    // Set offsets. These are used to correct for errors in the magnetometer readings. This 
    // is application dependent so it is part of the device init and not integrated into the 
    // driver/library. 
    // 
    // Calibration steps: 
    // 1. Set the below values to zero. 
    // 2. Make sure LSM303AGR_TEST_HEADING is enabled and build the project. 
    // 3. Connect the LSM303AGR to the STM32, connect the STM32F4 to your machine, open PuTTy 
    //    so you can see the output and flash the code. 
    // 4. Using a trusted compasss (such as the one on your phone), align it in the North 
    //    direction, then align the long edge of the LSM303AGR (X-axis) with the compass so 
    //    they're both pointing North. 
    // 5. Observe the output of the magnetometer via Putty (Note that depending on the compass 
    //    you use you may have to move it away from the magnetometer once it's aligned or else 
    //    else you could get magnetometer interference - this happens with phone compasses). 
    //    Note the difference between the magnetometer output and the compass heading and 
    //    record it in offsets[0] below. If the magnetometer reading is clockwise of the compass 
    //    heading then the value recorded will be negative. Recorded offsets are scaled by 10. 
    //    Compass and magnetometer heading are from 0-359 degrees. For example, if the compass 
    //    reads 0 degrees (Magnetic North) and the magnetometer output reads +105 (10.5 degrees) 
    //    then the offset recorded is -105. 
    // 6. Repeat steps 4 and 5 for all directions in 45 degree increments (NE, E, SE, etc.) and 
    //    record each subsequent direction in the next 'offsets' element. 

    offsets[0] = -100;    // N  (0/360deg) direction heading offset (degrees * 10) 
    offsets[1] = -20;     // NE (45deg) direction heading offset (degrees * 10) 
    offsets[2] = 0;       // E  (90deg) direction heading offset (degrees * 10) 
    offsets[3] = 0;       // SE (135deg) direction heading offset (degrees * 10) 
    offsets[4] = 180;     // S  (180deg) direction heading offset (degrees * 10) 
    offsets[5] = 200;     // SW (225deg) direction heading offset (degrees * 10) 
    offsets[6] = 140;     // W  (270deg) direction heading offset (degrees * 10) 
    offsets[7] = -90;     // NW (315deg) direction heading offset (degrees * 10) 

    // Driver init 
    lsm303agr_init(
        I2C1, 
        offsets, 
        LSM303AGR_M_ODR_10, 
        LSM303AGR_M_MODE_CONT, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE, 
        LSM303AGR_CFG_DISABLE); 
    
    //==================================================
} 


// Test code 
void lsm303agr_test_app(void)
{
    // Test code for the gpio_test here 

#if LSM303AGR_TEST_HEADING 
    // Update the magnetometer data 
    lsm303agr_m_read(); 

#if LSM303AGR_TEST_AXIS 
    lsm303agr_m_get_data(&mx_data, &my_data, &mz_data); 

    // Display the first device results - values are scaled to remove decimal 
    uart_sendstring(USART2, "mx = ");
    uart_send_integer(USART2, mx_data);
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "my = ");
    uart_send_integer(USART2, my_data); 
    uart_send_spaces(USART2, UART_SPACE_2);

    uart_sendstring(USART2, "mz = ");
    uart_send_integer(USART2, mz_data);
    uart_send_new_line(USART2); 
#endif   // LSM303AGR_TEST_AXIS 

    uart_sendstring(USART2, "heading = ");
    uart_send_integer(USART2, lsm303agr_m_get_heading());
    uart_send_spaces(USART2, UART_SPACE_2); 

    // Delay 
    tim_delay_ms(TIM9, 150);

#if LSM303AGR_TEST_AXIS 
    // Go up a line in the terminal to overwrite old data 
    uart_sendstring(USART2, "\033[1A"); 
#endif   // LSM303AGR_TEST_AXIS 

    // Go to a the start of the line in the terminal 
    uart_sendstring(USART2, "\r"); 
#endif   // LSM303AGR_TEST_HEADING 

#if LSM303AGR_TEST_NAV 
#endif   //LSM303AGR_TEST_NAV 
}


#if LSM303AGR_TEST_NAV 

// True North calibration - performed as requested (system state) 


// Format M8Q GPS readings 


// GPS coordinate radius check - done once after an updated location is read 
uint8_t lsm303agr_gps_rad(
    double *gps1, 
    double *gps2, 
    double surf_dist_thresh)
{
    // Local variables 
    double lat1 = *gps1++; 
    double lon1 = *gps1; 
    double lat2 = *gps2++; 
    double lon2 = *gps2; 
    double surf_dist = CLEAR; 
    uint8_t gps_rad = CLEAR; 
    double eq1, eq2, eq3, eq4, eq5; 

    // TODO the cooredinates must either be passed as radians or converted to radians 

    eq1 = cos(LSM303AGR_TEST_90DEG - lat2)*sin(lon2 - lon1); 
    eq2 = cos(LSM303AGR_TEST_90DEG - lat1)*sin(LSM303AGR_TEST_90DEG - lat2); 
    eq3 = sin(LSM303AGR_TEST_90DEG - lat1)*cos(LSM303AGR_TEST_90DEG - lat2)*cos(lon2 - lon1); 
    eq4 = sin(LSM303AGR_TEST_90DEG - lat1)*sin(LSM303AGR_TEST_90DEG - lat2); 
    eq5 = cos(LSM303AGR_TEST_90DEG - lat1)*cos(LSM303AGR_TEST_90DEG - lat2)*cos(lon2 - lon1); 

    // atan2 is used because it produces an angle between +/-180 (pi) and the central angle 
    // should always be positive and never greater than 180 
    surf_dist = atan2(sqrt((eq2 - eq3)*(eq2 - eq3) + (eq1*eq1)), (eq4 + eq5)) * 
                LSM303AGR_TEST_EARTH_R*LSM303AGR_TEST_KM_TO_M; 

    if (surf_dist < surf_dist_thresh)
    {
        gps_rad = SET_BIT; 
    }

    return gps_rad; 
}


// Desired heading calculation - done once after an updated location is read 
int16_t lsm303agr_heading_calc(
    double *gps1, 
    double *gps2)
{
    // Local variables 
    double lat1 = *gps1++; 
    double lon1 = *gps1; 
    double lat2 = *gps2++; 
    double lon2 = *gps2; 
    double num, den; 
    double heading_temp = CLEAR; 
    int16_t heading = CLEAR; 

    // Calculate the numerator and denominator of the atan calculation 
    num = cos(lat2)*sin(lon2-lon1); 
    den = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lon2-lon1); 

    // Calculate the heading 
    heading_temp = atan(num/den); 

    // Convert heading to degrees 
    heading = (int16_t)(heading_temp*10*180/3.14159); 

    // Correct the calculated heading if needed 
    if (den < 0)
    {
        heading += LSM303AGR_M_HEAD_DIFF; 
    }
    else if (num < 0)
    {
        heading += LSM303AGR_M_HEAD_MAX; 
    }

    return heading; 
}


// Heading error - done every heading update (~10Hz) 
int16_t lsm303agr_heading_error(
    int16_t heading_desired, 
    int16_t heading_current)
{
    // Local variables 
    int16_t heading_error = CLEAR; 

    // Calculate the heading error 
    heading_error = heading_desired - heading_current; 

    // Correct the error for when the heading crosses the 0/360 degree boundary 
    if (heading_error > LSM303AGR_M_HEAD_DIFF)
    {
        heading_error -= LSM303AGR_M_HEAD_MAX; 
    }
    else if (heading_error < -LSM303AGR_M_HEAD_DIFF)
    {
        heading_error += LSM303AGR_M_HEAD_MAX; 
    }

    return heading_error; 
}


// Motor controller - called after evry heading update 

#endif   // LSM303AGR_TEST_NAV 
