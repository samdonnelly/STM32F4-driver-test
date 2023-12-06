/**
 * @file project_init.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Project initialization code interface 
 * 
 * @version 0.1
 * @date 2023-07-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _PROJECT_INIT_H_ 
#define _PROJECT_INIT_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Include files are added to project_init.cpp so main.c can be used with C++. 

//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief Project initialization code - called once 
 */
void project_init(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _PROJECT_INIT_H_ 
