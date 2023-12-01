/**
 * @file m8q_location_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief M8Q GPS module - location testing interface 
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

#ifndef _M8Q_LOCATION_TEST_H_ 
#define _M8Q_LOCATION_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

// Temporary 
#include "m8q_test.h" 

//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief M8Q location test setup code 
 */
void m8q_location_test_init(void); 


/**
 * @brief Switch debounce test code 
 */
void m8q_location_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _M8Q_LOCATION_TEST_H_ 
