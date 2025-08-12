/**
 * @file driver_test_interface.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Driver test initialization and application code interface 
 * 
 * @version 0.1
 * @date 2023-07-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "driver_test_interface.h" 

//=======================================================================================


//=======================================================================================
// Functions 

// Driver test initialization code - called once 
void DriverTestInit(void)
{
    // Driver test initialization code here 
    dead_reckoning.TestInit(); 
}


// Driver test application code - called repeatedly 
void DriverTestApp(void)
{
    // Driver test application code here 
    dead_reckoning.TestApp(); 
}

//=======================================================================================
