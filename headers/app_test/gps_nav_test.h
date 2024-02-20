/**
 * @file gps_nav_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPS navigation testing interface 
 * 
 * @details The purpose of this code is to test navigation between predefined waypoints 
 *          using a compass (magnetometer) and a GNSS module. Once the GNSS device has a 
 *          position lock, the code reads the current location and compares it against 
 *          the first target location/waypoint by finding the distance and initial heading 
 *          between the two points. The compass heading is read which is then compared 
 *          to the heading between the two GNSS locations to find a heading error. The 
 *          heading error will indicate how much and in what direction the compass needs 
 *          to turn to be pointing at the target. The distance will indicate how close 
 *          the GNSS device is to the target. Using this, the target location can be 
 *          navigated to. Once the GNSS device has come within a certain radius of the 
 *          target, then the target will update to the next predefined waypoint. This 
 *          information will be output to a serial terminal for the user to see. 
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
