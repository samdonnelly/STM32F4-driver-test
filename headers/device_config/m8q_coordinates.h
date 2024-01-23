/**
 * @file m8q_coordinates.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief SAM-M8Q GPS coordinates file interface 
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

#ifndef _M8Q_COORDINATES_H_ 
#define _M8Q_COORDINATES_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "nav_calcs.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define M8Q_NUM_WAYPOINTS_0 4 

//=======================================================================================


//=======================================================================================
// Coordinates 

extern const gps_waypoints_t waypoints_0[M8Q_NUM_WAYPOINTS_0]; 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _M8Q_COORDINATES_H_
