/**
 * @file m8q_location_test.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief M8Q GPS module - location testing implementation 
 * 
 * @details The purpose of this code is to test the devices position against predefined 
 *          waypoints. This code reads the devices current location and calculates the 
 *          surface distance to a target waypoint. If the distance is less than a threshold 
 *          then the device is considered to have hit the waypoint at which point the next 
 *          waypoint is selected and the process repeats. The distance to the next waypoint 
 *          is displayed on a screen for user feedback. The is no indication of direction 
 *          to the next waypoint other that the current distance to it. The distance will 
 *          only be updated as often as the device sends new position data (once per second). 
 *          If there is no position lock by the device then the code will wait until there 
 *          is a lock before doing an waypoint calculations.
 * 
 * @version 0.1
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "m8q_location_test.h" 

//=======================================================================================


//=======================================================================================
// Classes 
//=======================================================================================


//=======================================================================================
// Setup code 

void m8q_location_test_init(void)
{
    // 
}

//=======================================================================================


//=======================================================================================
// Test code 

void m8q_location_test_app(void)
{
    // 
}

//=======================================================================================




//=======================================================================================
// Old code 

//==================================================
// Global variables 

// Sample waypoints 
static m8q_test_waypoints_t waypoints[M8Q_TEST_NUM_WAYPOINTS] = 
{
    {50.962010, -114.065890}, 
    {50.962340, -114.065930}, 
    {50.962340, -114.066260}, 
    {50.961730, -114.066080} 
}; 

// Target waypoint 
static m8q_test_waypoints_t waypoint; 

// Screen message buffer 
static char screen_msg[20]; 

//==================================================


//==================================================
// Prototypes 

// Setup code 
void m8q_test_init_copy(); 


// Test code 
void m8q_test_app_copy(); 


// GPS coordinate radius check - calculate surface distance and compare to threshold 
int16_t m8q_test_gps_rad_copy(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2); 


// GPS heading calculation 
int16_t m8q_test_gps_heading_copy(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2); 

//==================================================


//==================================================
// Setup code 

void m8q_test_init_copy()
{
    // Initialize GPIO ports 
    gpio_port_init(); 

    // Initialize timers 
    tim_9_to_11_counter_init(
        TIM9, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(TIM9); 

    // Initialize UART
    uart_init(
        USART2, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_DMA_DISABLE, 
        UART_DMA_DISABLE); 

    // Initialize I2C
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

    // HD44780U screen driver setup. Used to provide user feedback. Note that is the 
    // screen is on the same I2C bus as the M8Q then the screen must be setup first 
    // to prevent the screen interfering with the bus. 
    hd44780u_init(I2C1, TIM9, PCF8574_ADDR_HHH); 
    hd44780u_clear(); 

    // M8Q device setup 

    // Send the configuration messages to configure the device settings. The M8Q has 
    // no flash to store user settings. Instead they're saved RAM which can only be 
    // powered until the onboard backup battery loses power. For this reason, settings 
    // must always be configured in setup. 
    char m8q_config_messages[M8Q_CONFIG_MSG_NUM][M8Q_CONFIG_MSG_MAX_LEN]; 
    m8q_config_copy(m8q_config_messages); 

    // Driver init 
    m8q_init(
        I2C1, 
        GPIOC, 
        PIN_10, 
        PIN_11, 
        M8Q_CONFIG_MSG_NUM, 
        M8Q_CONFIG_MSG_MAX_LEN, 
        (uint8_t *)m8q_config_messages[0]); 
    
    // Output an initialization warning if a driver fault occurs on setup. 
    if (m8q_get_status()) 
    {
        uart_sendstring(USART2, "M8Q init fault.\r\n"); 
    }

    // Delay to let everything finish setup before starting to send and receieve data 
    tim_delay_ms(TIM9, 500); 
} 

//==================================================


//==================================================
// Test code 

void m8q_test_app_copy()
{
    // Local variables 
    uint8_t run = CLEAR; 
    double lat_current = CLEAR; 
    double lon_current = CLEAR; 
    int16_t radius = CLEAR; 
    int16_t heading = CLEAR; 
    static uint8_t waypoint_index = CLEAR; 
    static uint8_t waypoint_status = SET_BIT; 

    // Once there is data available on the device, read all messages. The device will 
    // update once per second. If there is data then the 'run' flag is set which 
    // triggers the rest of the test code. 
    while (m8q_get_tx_ready())
    {
        m8q_read(); 
        run++; 
    }

    // If the run flag is not zero then that means new data has been read from the 
    // device and the rest of the test code can be executed. 
    if (run)
    {
        // Check the position lock status. Relative position to waypoints can only be 
        // determined with a position lock so this is a requirement before doing 
        // any calculation. 
        if (m8q_get_navstat() == M8Q_NAVSTAT_G3)
        {
            // Position found. Proceed to determine the distance between the 
            // devices current location and the next waypoint. 

            // Get the updated location 
            lat_current = m8q_get_lat(); 
            lon_current = m8q_get_long(); 

            // If the device is close enough to a waypoint then the next waypoint in the 
            // mission is selected. 'waypoint_status' indicates when it's time to read 
            // the next waypoint. 
            if (waypoint_status)
            {
                // Update the target waypoint 
                waypoint.lat = waypoints[waypoint_index].lat; 
                waypoint.lon = waypoints[waypoint_index].lon; 

                // The status will be set again if the device hits (gets close enough to) 
                // the current target waypoint. 
                waypoint_status = CLEAR; 

                // Adjust waypoint index. If the end of the waypoint mission is reached 
                // then start over from the beginning. 
                if (++waypoint_index == 4)
                {
                    waypoint_index = CLEAR; 
                }
            }

            // Update GPS radius. This is the surface distance (arc distance) between 
            // the target waypoint and the current location. The radius is expressed 
            // in meters*10. 
            radius = m8q_test_gps_rad_copy(
                lat_current, 
                lon_current, 
                waypoint.lat, 
                waypoint.lon); 

            // Update the heading between the current location and the waypoint. The 
            // heading will be an angle between 0-359.9 degrees from true North in the 
            // clockwise direction between the devices current location and the 
            // waypoint location. The heading is expressed as degrees*10. 
            heading = m8q_test_gps_heading_copy(
                lat_current, 
                lon_current, 
                waypoint.lat, 
                waypoint.lon); 

            // Display the radius to the screen. 
            snprintf(screen_msg, 20, "Radius: %um   ", radius); 
            hd44780u_line_set(HD44780U_L1, screen_msg, HD44780U_CURSOR_NO_OFFSET); 
            snprintf(screen_msg, 20, "Heading: %udeg", heading); 
            hd44780u_line_set(HD44780U_L2, screen_msg, HD44780U_CURSOR_NO_OFFSET); 
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L1); 
            hd44780u_cursor_pos(HD44780U_START_L2, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L2); 

            // Check the radius against a threshold. If the radius is less than the 
            // threshold then the waypoint is considered to be hit and the 
            // 'waypoint_status' can be set to indicate that the next waypoint should 
            // be used. The radius and threshold are expressed in meters * 10. 
            if (radius < 100)
            {
                // Indicate that a new waypoint needs to be read 
                waypoint_status = SET_BIT; 
            }
        }
        else 
        {
            // No position lock. Display the status on the screen but do nothing else 
            // while there is no lock. 
            hd44780u_clear(); 
            hd44780u_line_set(HD44780U_L1, "No connection", HD44780U_CURSOR_NO_OFFSET); 
            hd44780u_cursor_pos(HD44780U_START_L1, HD44780U_CURSOR_NO_OFFSET);
            hd44780u_send_line(HD44780U_L1); 
        }
    }
}

//==================================================


//==================================================
// Helper functions 

// GPS coordinate radius check - calculate surface distance and compare to threshold 
int16_t m8q_test_gps_rad_copy(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2)
{
    // Local variables 
    int16_t gps_rad = CLEAR; 
    static double surf_dist = CLEAR; 
    double eq1, eq2, eq3, eq4, eq5; 
    double deg_to_rad = 3.14159/180; 
    double pi_over_2 = 3.14159/2.0; 
    double earth_rad = 6371; 
    double km_to_m = 1000; 

    // Convert coordinates to radians 
    lat1 *= deg_to_rad; 
    lon1 *= deg_to_rad; 
    lat2 *= deg_to_rad; 
    lon2 *= deg_to_rad; 

    eq1 = cos(pi_over_2 - lat2)*sin(lon2 - lon1); 
    eq2 = cos(pi_over_2 - lat1)*sin(pi_over_2 - lat2); 
    eq3 = sin(pi_over_2 - lat1)*cos(pi_over_2 - lat2)*cos(lon2 - lon1); 
    eq4 = sin(pi_over_2 - lat1)*sin(pi_over_2 - lat2); 
    eq5 = cos(pi_over_2 - lat1)*cos(pi_over_2 - lat2)*cos(lon2 - lon1); 

    // atan2 is used because it produces an angle between +/-180 (pi). The central angle 
    // should always be positive and never greater than 180. 
    // Calculate the radius using a low pass filter to smooth the data. 
    surf_dist += ((atan2(sqrt((eq2 - eq3)*(eq2 - eq3) + (eq1*eq1)), (eq4 + eq5)) * 
                 earth_rad*km_to_m) - surf_dist)*0.5; 
    gps_rad = (int16_t)(surf_dist*10); 

    return gps_rad; 
}


// GPS heading calculation 
int16_t m8q_test_gps_heading_copy(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2)
{
    // Local variables 
    int16_t heading = CLEAR; 
    static double heading_temp = CLEAR; 
    double num, den; 
    double deg_to_rad = 3.14159/180; 

    // Convert coordinates to radians 
    lat1 *= deg_to_rad; 
    lon1 *= deg_to_rad; 
    lat2 *= deg_to_rad; 
    lon2 *= deg_to_rad; 

    // Calculate the numerator and denominator of the atan calculation 
    num = cos(lat2)*sin(lon2-lon1); 
    den = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(lon2-lon1); 

    // Calculate the heading between coordinates. 
    // A low pass filter is used to smooth the data. 
    heading_temp += (atan(num/den) - heading_temp)*0.5; 

    // Convert heading to degrees 
    heading = (int16_t)(heading_temp*10/deg_to_rad); 

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

//==================================================

//=======================================================================================
