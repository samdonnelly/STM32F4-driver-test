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
#include "string.h"
#include "stdio.h"

//=======================================================================================


//=======================================================================================
// Function prototypes 

// To find the size of data in the buffer 
int buff_size(uint8_t *buff); 

// Clear the buffer 
void buff_clear(void); 

//=======================================================================================


//=======================================================================================
// FATFS variables 

FATFS   file_sys;            // File system 
FIL     file;                // File 
FRESULT fresult;             // Store the result of each operation 
char    buffer[BUFF_SIZE];   // To store the data that we can read or write
UINT    br, bw;              // Stores the counter to the read and write in the file 
BYTE    work[4096]; 
UINT    len = sizeof(work);  // 
// DWORD   cluster = 32768;     // 

// Capacity related variables 
FATFS    *pfs; 
DWORD    fre_clust; 
uint32_t total, free_space; 

DWORD clust = 512; 

// Debugging 
volatile uint8_t fail_state = 255; 
volatile uint8_t func_num[30]; 
volatile uint8_t mount_seq[100]; 
volatile uint8_t mount_it = 0; 
volatile uint8_t f_mount_steps[10];

//=======================================================================================


// User application 
void user_app()
{
    // Looped code here

    // Local variables 
    static uint8_t count = 1; 

    // Run once - code is here to put everything in the same file 
    if (count)
    {
        // Short delay to let the system set up 
        tim9_delay_ms(500); 

        // 
        disk_initialize(0); 

        // Mount the SD card 
        fresult = f_mount(&file_sys, "", 1); 
        if (fresult != FR_OK) uart2_sendstring("Error in mounting SD Card.\r\n");
        else uart2_sendstring("SD Card mounted successfully.\r\n"); 

        mount_seq[mount_it] = 255;

        count = 0; 
    }

    // Delay 
    tim9_delay_ms(1);
}


// To find the size of data in the buffer 
int buff_size(uint8_t *buff)
{
    int i = 0; 
    while(*buff++ != '\0') i++; 
    return i; 
}


// Clear the buffer 
void buff_clear(void)
{
    for(int i = 0; i < BUFF_SIZE; i++)
    {
        buffer[i] = '\0';
    }
}
