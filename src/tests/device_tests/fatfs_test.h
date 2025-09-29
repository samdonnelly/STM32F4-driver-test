/**
 * @file fatfs_test.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FATFS driver test interface 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _FATFS_TEST_H_
#define _FATFS_TEST_H_

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
 * @brief FATFS setup code 
 */
void fatfs_test_init(void); 


/**
 * @brief FATFS test code 
 */
void fatfs_test_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif  // _FATFS_TEST_H_ 
