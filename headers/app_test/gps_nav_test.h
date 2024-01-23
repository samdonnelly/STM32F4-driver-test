/**
 * @file gps_nav_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPS navigation testing interface 
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

#ifndef _GPS_NAV_TEST_H_ 
#define _GPS_NAV_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Macros 

#define GPS_NAV_TEST_SCREEN_ON_BUS 1    // HD44780U screen on same I2C bus as device 

//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief M8Q location test setup code 
 */
void gps_nav_test_init(void); 


/**
 * @brief M8Q location test code 
 */
void gps_nav_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _GPS_NAV_TEST_H_ 
