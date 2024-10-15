/**
 * @file wheel_rpm_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Wheel RPM (revolutions per minute) test interface 
 * 
 * @details This test determines the RPM of a wheel using a Hall Effect sensor and a 
 *          magnet. 
 * 
 * @version 0.1
 * @date 2024-10-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _WHEEL_RPM_TEST_H_ 
#define _WHEEL_RPM_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Function prototypes 

/**
 * @brief Wheel RPM test setup code 
 */
void wheel_rpm_test_init(void); 


/**
 * @brief Wheel RPM test application code 
 */
void wheel_rpm_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _WHEEL_RPM_TEST_H_ 
