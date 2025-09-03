/**
 * @file gps_coordinates.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief GPS coordinates file interface 
 *        
 *        Note: the implementation of this header is not tracked because coordinates 
 *              are use case specific. 
 * 
 * @version 0.1
 * @date 2024-01-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _GPS_COORDINATES_H_ 
#define _GPS_COORDINATES_H_ 

//=======================================================================================
// Includes 

#include "nav_calcs.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define NUM_GPS_WAYPOINTS_0 4 

//=======================================================================================


//=======================================================================================
// Coordinates 

extern const NavCalcs::Position waypoints_0[NUM_GPS_WAYPOINTS_0]; 

//=======================================================================================

#endif  // _GPS_COORDINATES_H_
