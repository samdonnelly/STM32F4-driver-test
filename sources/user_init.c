/**
 * @file user_init.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief Code that gets called once at the beginning of the program
 * 
 * @version 0.1
 * @date 2022-02-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "user_init.h"

//=======================================================================================


// User init function 
void user_init()
{
    // Setup code here

    // Local variables 
    uint8_t card_type; 

    // Initialize timers 
    tim9_init(TIMERS_APB2_84MHZ_1US_PRESCALAR);

    // Communication initialization
    uart2_init(UART2_BAUD_9600); 
    spi2_init(SPI2_1_SLAVE, BR_FPCLK_2, SPI_CLOCK_MODE_0);

    // Device initialization 
    card_type = hw125_init(GPIO_PIN_12);

    // Check the SD card initialization status 
    switch (card_type)
    {
        case HW125_CT_UNKNOWN:  // Error occured 
            uart2_sendstring("Error: Unknown card\r\n");
            break;
        case HW125_CT_MMC:  // MMC V3 
            uart2_sendstring("MMC V3\r\n");
            break;
        case HW125_CT_SDC1:  // SDC V1 
            uart2_sendstring("SDC V1\r\n");
            break;
        case HW125_CT_SDC2_BLOCK:  // SDC V2 block 
            uart2_sendstring("SDC V2 block\r\n");
            break;
        case HW125_CT_SDC2_BYTE:  // SDC V2 byte 
            uart2_sendstring("SDC V2 byte\r\n");
            break;
        default:  // Unknown response 
            uart2_sendstring("Unknown response\r\n");
            break;
    }
}
