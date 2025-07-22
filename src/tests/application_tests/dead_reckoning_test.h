/**
 * @file dead_reckoning_test.h
 * 
 * @author your name (you@domain.com)
 * 
 * @brief Dead reckoning test interface 
 * 
 * @version 0.1
 * @date 2025-07-22
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef _DEAD_RECKONING_H_ 
#define _DEAD_RECKONING_H_ 

//=======================================================================================
// Includes 

// #include "project.h" 

//=======================================================================================


//=======================================================================================
// 

class IProjectInterface; 

class DeadReckoningTest final : public IProjectInterface 
{
public: 
    // 
    void ProjectInit(void) override; 

    // 
    void ProjectApp(void) override; 
};

DeadReckoningTest &dead_reckoning_test; 

//=======================================================================================

#endif   // _DEAD_RECKONING_H_ 
