/**
 * @file rpm_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief RPM (revolutions per minute) test interface 
 * 
 * @version 0.1
 * @date 2024-10-12
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _RPM_TEST_H_ 
#define _RPM_TEST_H_ 

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
 * @brief RPM test setup code 
 */
void rpm_test_init(void); 


/**
 * @brief RPM test application code 
 */
void rpm_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _RPM_TEST_H_ 
