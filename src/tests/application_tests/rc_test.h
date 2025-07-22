/**
 * @file rc_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Remote control (RC) test interface 
 * 
 * @version 0.1
 * @date 2024-05-05
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef _RC_TEST_H_ 
#define _RC_TEST_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

#include "includes_drivers.h" 

//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief Remote control test setup code 
 */
void rc_test_init(void); 


/**
 * @brief Remote control test code 
 */
void rc_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _RC_TEST_H_ 
