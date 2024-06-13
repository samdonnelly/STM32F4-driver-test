/**
 * @file project_init.cpp
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Project initialization code 
 * 
 * @version 0.1
 * @date 2023-07-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

//=======================================================================================
// Includes 

#include "project_interface.h" 

#include "includes_app.h"
#include "includes_cpp_app.h"
#include "includes_drivers.h"
#include "includes_cpp_drivers.h"

//=======================================================================================


//=======================================================================================
// Functions 

// Project initialization code - called once 
void project_init(void)
{
    // Project initialization code here 

    // rc_test_init(); 
    nrf24l01_test_init(); 
}

//=======================================================================================
