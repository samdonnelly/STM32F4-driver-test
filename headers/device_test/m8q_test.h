/**
 * @file m8q_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q test code header 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _M8Q_TEST_H_
#define _M8Q_TEST_H_

//=======================================================================================
// Includes 

#include "includes_drivers.h"
#include "int_handlers.h"

// Test code 
#include "state_machine_test.h" 

//=======================================================================================


//=======================================================================================
// Macros 

// Conditional compilation 
#define M8Q_CONTROLLER_TEST 0        // Choose between driver and controller test code 
#define M8Q_TEST_SCREEN_ON_BUS 1     // HD44780U screen on I2C bus 

// Controller testing 
#define M8Q_NUM_USER_CMDS 17         // Number of defined user commands 
#define M8Q_MAX_SETTER_ARGS 1        // Maximum arguments of all function pointer below 

// Location testing 
#define M8Q_TEST_NUM_WAYPOINTS 4     // Number of waypoints for location testing 
#define M8Q_TEST_CALC_SCALE 10       // Scalar for calculated data 
#define M8Q_TEST_PI_RAD 3.14159      // PI 
#define M8Q_TEST_180_DEG 180         // 180 degrees 
#define M8Q_TEST_EARTH_RAD 6371      // Earch average radius (km) 
#define M8Q_TEST_KM_TO_M 1000        // Km to m conversion 
#define M8Q_TEST_HEADING_GAIN 0.5    // GPS heading low pass filter gain 
#define M8Q_TEST_RADIUS_GAIN 0.5     // GPS radius low pass filter gain 

//=======================================================================================


//=======================================================================================
// Function pointers 

/**
 * @brief M8Q line clear and state flag function pointer 
 * 
 * @details This function pointer is used for calling the following setters from the device 
 *          controller: 
 *           - m8q_set_read_ready 
 *           - m8q_clear_read_ready 
 *           - m8q_set_read_flag 
 *           - m8q_clear_read_flag 
 *           - m8q_set_low_pwr_flag 
 *           - m8q_clear_low_pwr_flag 
 *           - m8q_set_reset_flag 
 */
typedef void (*m8q_setter_ptr_1)(void); 


/**
 * @brief M8Q getters function pointer 1 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_state
 */
typedef M8Q_STATE (*m8q_getter_ptr_1)(void); 


/**
 * @brief M8Q getters function pointer 2 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_fault_code 
 *           - m8q_get_navstat 
 */
typedef uint16_t (*m8q_getter_ptr_2)(void); 


/**
 * @brief M8Q getters function pointer 3 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_lat_str 
 *           - m8q_get_long_str 
 */
typedef void (*m8q_getter_ptr_3)(
    uint8_t *deg_min, 
    uint8_t *min_frac); 


/**
 * @brief M8Q getters function pointer 4 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_NS 
 *           - m8q_get_EW 
 */
typedef uint8_t (*m8q_getter_ptr_4)(void); 


/**
 * @brief M8Q getters function pointer 5 
 * 
 * @details This function pointer is used for calling the following getters from the device 
 *          controller: 
 *           - m8q_get_time 
 *           - m8q_get_date 
 */
typedef void (*m8q_getter_ptr_5)(
    uint8_t *utc); 

//=======================================================================================


//=======================================================================================
// Structures 

/**
 * @brief Structure of all M8Q setter and getter function pointers (see above) 
 */
typedef struct m8q_func_ptrs_s 
{
    m8q_setter_ptr_1 setter_1; 
    m8q_getter_ptr_1 getter_1; 
    m8q_getter_ptr_2 getter_2; 
    m8q_getter_ptr_3 getter_3; 
    m8q_getter_ptr_4 getter_4; 
    m8q_getter_ptr_5 getter_5; 
}
m8q_func_ptrs_t; 

//=======================================================================================


//=======================================================================================
// Setup code 

/**
 * @brief Setup code for Test 0 
 */
void m8q_test_0_init(void); 


/**
 * @brief Setup code for Test 1 
 */
void m8q_test_1_init(void); 


/**
 * @brief Setup code for Test 2 
 */
void m8q_test_2_init(void); 

//=======================================================================================


//=======================================================================================
// Test code 

/**
 * @brief Test 0 code 
 * 
 * @details No config messages are sent to the device during setup so the device operates 
 *          in its default configuration (assuming no configurations saved in RAM). The 
 *          whole message stream is read and output over UART (to serial terminal). a 
 *          data buffer limit is set and neither TX ready or low power pins are used. 
 *          While the device is active, the code periodically stops reading the stream 
 *          and lets the buffer overflow. After an overflow, the data buffer is flushed 
 *          and reading continues. Periodically the device is put into and taken out of 
 *          low power mode via a sent messge. 
 *          
 *          Note that during periods where the device is not in low power mode but not 
 *          actively reading data ("read pause"), the data size is read to keep the I2C 
 *          port of the device active. If this is not done the device will asssume the 
 *          host/master is no longer using this interface and it will stop scheduling 
 *          data packets to the port. By keeping the port active, the data stream will 
 *          accumulate past the max data buffer size and a data stream flush (buffer 
 *          overflow) will be triggers. In other words, this is all done to demonstrate 
 *          how the dirver works. 
 */
void m8q_test_0(void); 


/**
 * @brief Test 1 code 
 * 
 * @details Config messages sent to the device and specific data getters are used to read data. 
 *          The code will periodically put the device to sleep via the low power pin then turn 
 *          on again and continue to read data. The read data gets output over UART (to serial 
 *          terminal). Data availability is checked via the TX ready pin. No data buffer limit 
 *          is set. 
 */
void m8q_test_1(void); 


/**
 * @brief Test 2 code 
 * 
 * @details M8Q controller test 
 */
void m8q_test_2(void); 

//=======================================================================================


//=======================================================================================
// Test functions 

/**
 * @brief GPS coordinate radius check 
 * 
 * @details Calculates the surface distance between the devices current location and the 
 *          target waypoint. The distance is returned in meters*10 (meters = radius/10). 
 *          The central angle between the devices location and the waypoint is found and
 *          used along with the average Earth radius to calculate the surface distance. 
 * 
 * @param lat1 : current device latitude 
 * @param lon1 : current device longitude 
 * @param lat2 : target waypoint latitude 
 * @param lon2 : target waypoint longitude 
 * @return int16_t : scaled surface distance between location and waypoint 
 */
int16_t m8q_test_gps_rad(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2); 


/**
 * @brief GPS heading calculation 
 * 
 * @details Calculates the heading between the current device location and the target 
 *          waypoint. The heading is an angle between 0-359.9 degrees from true North in 
 *          the clockwise direction. The return value is the heading expressed in 
 *          degrees*10. 
 * 
 * @param lat1 : current device latitude 
 * @param lon1 : current device longitude 
 * @param lat2 : target waypoint latitude 
 * @param lon2 : target waypoint longitude 
 * @return int16_t : scaled 0-360 degree true North heading 
 */
int16_t m8q_test_gps_heading(
    double lat1, 
    double lon1, 
    double lat2, 
    double lon2); 

//=======================================================================================

#endif  // _M8Q_TEST_H_ 
