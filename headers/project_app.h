/**
 * @file project_app.h
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Project application code interface 
 * 
 * @version 0.1
 * @date 2023-07-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef _PROJECT_APP_H_ 
#define _PROJECT_APP_H_ 

#ifdef __cplusplus
extern "C" {
#endif

//=======================================================================================
// Includes 

// Include files are added to project_app.cpp so main.c can be used with C++. 

//=======================================================================================


//=======================================================================================
// Functions 

/**
 * @brief Project application code - called repeatedly 
 */
void project_app(void); 

//=======================================================================================

#ifdef __cplusplus
}
#endif

#endif   // _PROJECT_APP_H_ 
