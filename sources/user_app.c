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
    // Looped code here
#if HC05_AT_CMD_MODE  // AT command code 
    uart2_send_new_line(); 
#else  // Data mode 
    uart2_send_new_line(); 
#endif  // HC05_AT_CMD_MODE
}
