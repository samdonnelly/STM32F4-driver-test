/**
 * @file user_app.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once per loop
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "user_app.h"

//=======================================================================================


// User application 
void user_app()
{
    //==============================================================
    // Control Code 
    //  1. Read temperature once at the start (see uaer_init)
    //  2. Read accelerometer data once per loop (keep in mind max speed of sensor)
    //  3. Display the data over serial for viewing
    //  4. Delay 
    //  5. Repeat 
    //==============================================================
    
    // Read from the accelerometer

    // Print data to serial terminal 

    // Delay 
    tim9_delay_ms(100);
}
