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

// Format drive variables 
BYTE    work[512];           // 

// Capacity related variables 
FATFS    *pfs; 
DWORD    fre_clust; 
uint32_t total, free_space; 

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

#if FORMAT_EXFAT
        // Format the drive 
        fresult = f_mkfs("", FM_EXFAT, 0, work, sizeof work); 
        if (fresult != FR_OK) uart2_sendstring("Error in formatting the SD Card.\r\n");
        else uart2_sendstring("SD Card formatted successfully.\r\n"); 
#endif

        //=============================================
        // Mount the SD card 

        fresult = f_mount(&file_sys, "", 1); 
        if (fresult != FR_OK) uart2_sendstring("Error in mounting SD Card.\r\n");
        else uart2_sendstring("SD Card mounted successfully.\r\n"); 

        //=============================================


        //=============================================
        // Card Capacity 

        // These calcs assume 512 bytes/sector 

        // Check free space 
        f_getfree("", &fre_clust, &pfs);

        total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
        sprintf (buffer, "SD CARD Total Size: \t%lu KB\r\n", total);
        uart2_sendstring(buffer);
        buff_clear();
        
        free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
        sprintf (buffer, "SD CARD Free Space: \t%lu KB\r\n\n", free_space);
        uart2_sendstring(buffer);
        buff_clear();

        //=============================================

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
