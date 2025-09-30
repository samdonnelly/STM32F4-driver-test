/**
 * @file fatfs_test.c
 * 
 * @author Sam Donnelly (samueldonnelly11@gmail.com)
 * 
 * @brief FATFS driver test 
 * 
 * @version 0.1
 * @date 2022-08-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

//=======================================================================================
// Includes 

#include "fatfs_test.h"
#include "stm32f4xx_it.h"

//=======================================================================================


//=======================================================================================
// Macros 

#define DATA_BUFF_SIZE 250     // Data buffer size 
#define FORMAT_BUFF_SIZE 512   // Size of buffer for formatting the volume 
#define CMD_SIZE 50            // Max user command string length 
#define NUM_USER_CMDS 21       // Number of driver test commands for the user 
#define FORMAT_ATTEMPTS 20     // Number of attempts at formatting the volume before timeout 

//=======================================================================================


//=======================================================================================
// Enums 

// User input type 
typedef enum {
    FORMAT_FILE_STRING, 
    FORMAT_FILE_MODE, 
    FORMAT_FILE_NUM 
} format_user_input_t; 

//=======================================================================================


//=======================================================================================
// Prototypes 

// User command functions 
void file_mkfs(void);         // Format the volume 
void mount_card(void);        // Mount the volume 
void unmount_card(void);      // Unmount the volume 
void card_capacity(void);     // Card Capacity 
void file_check(void);        // Check existance of a file or sub-directory 
void file_remove(void);       // Remove files from the drive 
void file_mkdir(void);        // Make a new directory 
void file_chdir(void);        // Change the current directory 
void file_getcwd(void);       // Retrieve the current directory and drive 
void file_opendir(void);      // Open a directory 
void file_closedir(void);     // Close an open directory 
void file_find(void);         // Find files on card 
void file_open(void);         // Open a file 
void file_close(void);        // Close the open file 
void file_read(void);         // Read from an open file using using f_read 
void file_write(void);        // Write to an open file using f_write 
void file_seek(void);         // Navigate the file 
void file_rewind(void);       // Navigate to the beginning of the file 
void file_fast_fwd(void);     // Navigate to the end of the file 
void file_gets(void);         // Read from an open file using f_gets 
void file_puts(void);         // Write to an open file using f_puts 

// Command control 
void cmd_select(void);                                             // Select command based on user input 
void cmd_reset(void);                                              // Return to default state at the end of the command dispatch 
void string_display(const char *str_format, const char *str_arg);  // Display string with string argument 
void number_display(const char *str_format, const DWORD num_arg);  // Display string with number argument 
void fault_display(void);                                          // Display the error code related to the FATFS operation 
void feedback_display(const char *string);                         // Display a string for the user to see 

// Format user input 
uint8_t format_input(char *buff, DWORD *data, format_user_input_t op);

//=======================================================================================


//=======================================================================================
// Global variables 

// Data record 
typedef struct fatfs_test_record_s 
{
    // Peripherals 
    SPI_TypeDef *spi;
    USART_TypeDef *uart;
    DMA_Stream_TypeDef *dma_stream;
    TIM_TypeDef *tim;

    // Serial interface data 
    uint8_t cb[DATA_BUFF_SIZE];              // Circular buffer populated by DMA 
    cb_index_t cb_index;                     // Circular buffer indexing info 
    dma_index_t dma_index;                   // DMA transfer indexing info 
    uint8_t data_in_buff[DATA_BUFF_SIZE];    // Buffer that stores latest UART input 
    uint8_t data_out_buff[DATA_BUFF_SIZE];   // Buffer that stores outgoing data 
    DWORD data_in_num;                       // Number fetched from data input 
    
    // State tracking 
    uint8_t state_seq;                       // Contorls action within a state/command 
    void (*state_func_ptr)(void);            // Dispatches to a chosen command 
    
    // File variables 
    FATFS file_sys;                          // File system 
    FIL file;                                // File 
    FRESULT fresult;                         // Store the result of each operation 
    UINT br, bw;                             // Stores f_read and f_write byte counters 
    DIR dj;                                  // Directory object 
    FILINFO fno;                             // File information 
} 
fatfs_test_record_t;

// Data record instance 
static fatfs_test_record_t fatfs_data;


// Command pointers 
typedef struct fatfs_user_cmds_s 
{
    char user_cmds[CMD_SIZE];              // Stores the defined user input commands 
    void (*fatfs_func_ptrs_t)(void);       // Pointer to FatFs file operation function 
}
fatfs_user_cmds_t;

// User commands 
static const fatfs_user_cmds_t cmd_table[NUM_USER_CMDS] = 
{
    // Volume Management and System Configuration commands 
    {"mount",     &mount_card},
    {"unmount",   &unmount_card},
    {"mkfs",      &file_mkfs},
    {"getfree",   &card_capacity},
    // File and Directory Management commands 
    {"stat",      &file_check},
    {"unlink",    &file_remove},
    {"mkdir",     &file_mkdir},
    {"chdir",     &file_chdir},
    {"getcwd",    &file_getcwd},
    // Directory Access commands 
    {"opendir",   &file_opendir},
    {"closedir",  &file_closedir},
    {"find",      &file_find},
    // File Access commands 
    {"openfile",  &file_open},
    {"closefile", &file_close},
    {"read",      &file_read},
    {"write",     &file_write},
    {"lseek",     &file_seek},
    {"rewind",    &file_rewind},
    {"fastfwd",   &file_fast_fwd},
    {"gets",      &file_gets},
    {"puts",      &file_puts}
};


// FatFs layer disk status - used for clearing the init status for re-mounting 
extern Disk_drvTypeDef disk;

//=======================================================================================


//=======================================================================================
// Setup code

void fatfs_test_init()
{
    // Initialize data 
    fatfs_data.spi = SPI2;
    fatfs_data.uart = USART2;
    fatfs_data.dma_stream = DMA1_Stream5;
    fatfs_data.tim = TIM9;

    memset((void *)fatfs_data.cb, CLEAR, sizeof(fatfs_data.cb)); 
    fatfs_data.cb_index.cb_size = DATA_BUFF_SIZE; 
    fatfs_data.cb_index.head = CLEAR; 
    fatfs_data.cb_index.tail = CLEAR; 
    fatfs_data.dma_index.data_size = CLEAR; 
    fatfs_data.dma_index.ndt_old = dma_ndt_read(fatfs_data.dma_stream); 
    fatfs_data.dma_index.ndt_new = CLEAR; 
    memset((void *)fatfs_data.data_in_buff, CLEAR, sizeof(fatfs_data.data_in_buff)); 
    memset((void *)fatfs_data.data_out_buff, CLEAR, sizeof(fatfs_data.data_out_buff)); 

    fatfs_data.state_seq = CLEAR;

    //==================================================
    // General setup 

    // Initialize GPIO ports 
    gpio_port_init(); 
    
    // Initialize timers 
    tim_9_to_11_counter_init(
        fatfs_data.tim, 
        TIM_84MHZ_1US_PSC, 
        0xFFFF,  // Max ARR value 
        TIM_UP_INT_DISABLE); 
    tim_enable(fatfs_data.tim);

    //==================================================

    //==================================================
    // UART 
    
    // UART2 init - Serial terminal 
    uart_init(
        fatfs_data.uart, 
        GPIOA, 
        PIN_3, 
        PIN_2, 
        UART_PARAM_DISABLE,    // Word length 
        CLEAR_BIT,             // STOP bits 
        UART_FRAC_42_9600, 
        UART_MANT_42_9600, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE); 
        
    // UART2 interrupt init - Serial terminal - IDLE line (RX) interrupts 
    uart_interrupt_init(
        fatfs_data.uart, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_ENABLE, 
        UART_PARAM_DISABLE, 
        UART_PARAM_DISABLE);
    
    //==================================================

    //==================================================
    // SPI 
    
    // SPI and slave select pin for SD card 
    spi_init(
        fatfs_data.spi, 
        GPIOB,   // SCK pin GPIO port 
        PIN_10,  // SCK pin 
        GPIOB,   // Data (MISO/MOSI) pin GPIO port 
        PIN_14,  // MISO pin 
        PIN_15,  // MOSI pin 
        SPI_BR_FPCLK_8, 
        SPI_CLOCK_MODE_0); 
    spi_ss_init(GPIOB, PIN_12);
    
    //==================================================

    //==================================================
    // DMA 

    // DMA1 stream init - UART2 - Serial terminal 
    dma_stream_init(
        DMA1, 
        fatfs_data.dma_stream, 
        DMA_CHNL_4, 
        DMA_DIR_PM, 
        DMA_CM_ENABLE,
        DMA_PRIOR_HI, 
        DMA_DBM_DISABLE, 
        DMA_ADDR_INCREMENT,   // Increment the buffer pointer to fill the buffer 
        DMA_ADDR_FIXED,       // No peripheral increment - copy from DR only 
        DMA_DATA_SIZE_BYTE, 
        DMA_DATA_SIZE_BYTE);
        
    // DMA1 stream config - UART2 - Serial terminal 
    dma_stream_config(
        fatfs_data.dma_stream, 
        (uint32_t)(&fatfs_data.uart->DR), 
        (uint32_t)fatfs_data.cb, 
        (uint32_t)NULL, 
        (uint16_t)DATA_BUFF_SIZE); 
        
    // Enable DMA streams 
    dma_stream_enable(fatfs_data.dma_stream);    // UART2 - Serial terminal 
    
    //==================================================

    //==================================================
    // Interrupts 

    // Initialize interrupt handler flags 
    int_handler_init(); 

    // Enable the interrupt handlers 
    nvic_config(USART2_IRQn, EXTI_PRIORITY_0);   // UART2 - Serial terminal (user input) 

    //==================================================

    //==================================================
    // SD card init 

    // SD card user initialization 
    fatfs_user_init(fatfs_data.spi, GPIOB, GPIOX_PIN_12); 
    
    //==================================================

    cmd_reset();
} 

//=======================================================================================


//=======================================================================================
// Test code 

void fatfs_test_app()
{
    // New serial terminal (user input) data received 
    if (handler_flags.usart2_flag)
    {
        handler_flags.usart2_flag = CLEAR_BIT;

        // Parse the new user message from the circular buffer into the data buffer 
        dma_cb_index(fatfs_data.dma_stream, &fatfs_data.dma_index, &fatfs_data.cb_index);
        cb_parse(fatfs_data.cb, &fatfs_data.cb_index, fatfs_data.data_in_buff);

        // Dispatch using function pointer 
        fatfs_data.state_func_ptr();
    }
}

//=======================================================================================


//=======================================================================================
// User command functions 

// Format the card 
void file_mkfs(void)
{
    uint8_t timer = FORMAT_ATTEMPTS;
    BYTE work[FORMAT_BUFF_SIZE];

    do
    {
        fatfs_data.fresult = f_mkfs("", FM_EXFAT, 0, work, sizeof(work));
        (fatfs_data.fresult == FR_OK) ? feedback_display("SD Card formatted successfully.\r\n") : 
                                        feedback_display("Error formatting volume.\r\n");
    }
    while (fatfs_data.fresult != FR_OK && --timer);

    cmd_reset();
}


// Mount card 
void mount_card(void)
{
    fatfs_data.fresult = f_mount(&fatfs_data.file_sys, "", FATFS_MOUNT_NOW); 

    if (fatfs_data.fresult == FR_OK) 
    {
        feedback_display("\nMounted successfully. Volume type: ");

        // Check the volume type 
        switch (fatfs_get_card_type())
        {
            case FATFS_CT_MMC: 
                feedback_display("MMC V3\r\n");
                break;
            case FATFS_CT_SDC1: 
                feedback_display("SDC V1\r\n");
                break;
            case FATFS_CT_SDC2_BLOCK: 
                feedback_display("SDC V2 block\r\n");
                break;
            case FATFS_CT_SDC2_BYTE: 
                feedback_display("SDC V2 byte\r\n");
                break;
            default: 
                feedback_display("Unknown\r\n");
                break;
        }
    }
    else 
    {
        fault_display();
    }

    cmd_reset();
}


// Unmount card 
void unmount_card(void) 
{
    // Unmount the volume and clear the initialization status so it can be re-mounted. 
    fatfs_data.fresult = f_unmount("");
    disk.is_initialized[0] = CLEAR;

    (fatfs_data.fresult == FR_OK) ? feedback_display("\nVolume unmounted successfully.\r\n") : 
                                    feedback_display("\nError in unmounting volume.\r\n");
    cmd_reset();
}


// Check card capacity 
void card_capacity(void)
{
    // These calcs assume 512 bytes/sector 

    FATFS *pfs;                // Pointer to file system object 
    DWORD fre_clust;           // Stores number of free clusters 

    // Check free space 
    f_getfree("", &fre_clust, &pfs);

    // Calculate the total space 
    DWORD total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize / 2);
    number_display("\nVolume Total Size (KB): ", total);
    
    // Calculate the free space 
    DWORD free_space = (uint32_t)(fre_clust * pfs->csize / 2);
    number_display("Volume Free Space (KB): ", free_space);

    cmd_reset();
}


// Check existance of a file or sub-directory 
void file_check(void)
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nFile or directory to check: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);

        // Check for the existance of the specified file or directory 
        fatfs_data.fresult = f_stat((TCHAR *)fatfs_data.data_in_buff, &fatfs_data.fno);

        switch(fatfs_data.fresult)
        {
            case FR_OK:
                string_display("\r\nExists: ", (char *)fatfs_data.data_in_buff);
                break;

            case FR_NO_FILE:
                string_display("\r\nDoes not exists: ", (char *)fatfs_data.data_in_buff);
                break;

            default:
                fault_display();
                break;
        }

        cmd_reset();
    }
}


// Remove files on card 
void file_remove(void) 
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nFile to remove: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);

        // Attempt to remove the specified file 
        fatfs_data.fresult = f_unlink((TCHAR *)fatfs_data.data_in_buff);
        (fatfs_data.fresult == FR_OK) ? string_display("\r\nSuccessfully removed: ", (char *)fatfs_data.data_in_buff) : 
                                        fault_display();
        cmd_reset();
    }
}


// Make a new directory 
void file_mkdir(void)
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nDirectory: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);

        // Write to the file 
        fatfs_data.fresult = f_mkdir((TCHAR *)fatfs_data.data_in_buff);
        (fatfs_data.fresult == FR_OK) ? string_display("\r\nNew directory: ", (char *)fatfs_data.data_in_buff) : 
                                        fault_display();
        cmd_reset();
    }
}


// Change the current directory 
void file_chdir(void)
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nDirectory to go to: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);

        // Change the directory 
        fatfs_data.fresult = f_chdir((TCHAR *)fatfs_data.data_in_buff);
        (fatfs_data.fresult == FR_OK) ? string_display("\r\nNow in: ", (char *)fatfs_data.data_in_buff) : 
                                        fault_display();
        cmd_reset();
    }
}


// Retrieve the current directory and drive 
void file_getcwd(void)
{
    if (fatfs_data.file_sys.fs_type == FS_EXFAT)
    {
        feedback_display("\r\nCan't fetch current directory of exFAT volumes. getcwd return root\r\n");
    }

    char path[DATA_BUFF_SIZE];

    // Get the current directory 
    fatfs_data.fresult = f_getcwd(path, DATA_BUFF_SIZE);
    (fatfs_data.fresult == FR_OK) ? string_display("\r\nCurrent directory: ", path) : 
                                    fault_display();
    cmd_reset();
}


// Open a directory 
void file_opendir(void)
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nDirectory to open: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);

        // Open the specified directory 
        fatfs_data.fresult = f_opendir(&fatfs_data.dj, (TCHAR *)fatfs_data.data_in_buff);
        (fatfs_data.fresult == FR_OK) ? string_display("\r\nOpened: ", (char *)fatfs_data.data_in_buff) : 
                                        fault_display();
        cmd_reset();
    }
}


// Close an open directory 
void file_closedir(void)
{
    // Close the current directory 
    fatfs_data.fresult = f_closedir(&fatfs_data.dj);
    (fatfs_data.fresult == FR_OK) ? feedback_display("\r\nDirectory closed.") : 
                                    fault_display();
    cmd_reset();
}


// Check files on the card 
void file_find(void)
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nPath: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);
        feedback_display("\nFiles in path: \r\n");

        // Start to search for files 
        fatfs_data.fresult = f_findfirst(&fatfs_data.dj, &fatfs_data.fno, fatfs_data.data_in_buff, "*");

        while ((fatfs_data.fresult == FR_OK) && fatfs_data.fno.fname[0])
        {
            uart_send_str(fatfs_data.uart, "\t- "); 
            uart_send_str(fatfs_data.uart, fatfs_data.fno.fname); 
            uart_send_new_line(fatfs_data.uart); 
            fatfs_data.fresult = f_findnext(&fatfs_data.dj, &fatfs_data.fno); 
        }

        f_closedir(&fatfs_data.dj);
        cmd_reset();
    }
}


// Open a file 
void file_open(void)
{
    static char path[DATA_BUFF_SIZE];

    if (fatfs_data.state_seq == 0)
    {
        feedback_display("\nFile to open: ");
        fatfs_data.state_seq++;
    }
    else if (fatfs_data.state_seq == 1)
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);
        strcpy(path, (char *)fatfs_data.data_in_buff);
        feedback_display("\nAccess mode: ");
        fatfs_data.state_seq++;
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_MODE);

        // Open a file (and create if it doesn't exist) 
        fatfs_data.fresult = f_open(&fatfs_data.file, path, (BYTE)fatfs_data.data_in_num);
        (fatfs_data.fresult == FR_OK) ? string_display("\r\nOpened: ", (char *)fatfs_data.data_in_buff) : 
                                        fault_display();
        cmd_reset();
    }
}


// Close the open file 
void file_close(void) 
{
    // Close the open file 
    fatfs_data.fresult = f_close(&fatfs_data.file);
    (fatfs_data.fresult == FR_OK) ? feedback_display("\r\nFile closed.") : 
                                    fault_display();
    cmd_reset();
}


// Read from an open file using using f_read 
void file_read(void) 
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nRead size (bytes): ");
    }
    else
    {
        char file_data[DATA_BUFF_SIZE];
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_NUM);

        // Read from the file 
        fatfs_data.fresult = f_read(&fatfs_data.file, 
                                    (void *)file_data, 
                                    fatfs_data.data_in_num, 
                                    &fatfs_data.br);
        (fatfs_data.fresult == FR_OK) ? string_display("\r\nFile data: ", file_data) : 
                                        fault_display();
        cmd_reset();
    }
}


// Write to an open file using f_write 
void file_write(void) 
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nFile data: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);

        // Write to the file 
        fatfs_data.fresult = f_write(&fatfs_data.file, 
                                     (void *)fatfs_data.data_in_buff, 
                                     strlen(fatfs_data.data_in_buff), 
                                     &fatfs_data.bw);
        (fatfs_data.fresult == FR_OK) ? string_display("\r\nFile data: ", (char *)fatfs_data.data_in_buff) : 
                                        fault_display();
        cmd_reset();
    }
}


// Navigate the file 
void file_seek(void) 
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nFile position: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_NUM);

        // Move to the specified position in the file 
        fatfs_data.fresult = f_lseek(&fatfs_data.file, fatfs_data.data_in_num);
        (fatfs_data.fresult == FR_OK) ? number_display("\r\nNew file position: ", fatfs_data.data_in_num) : 
                                        fault_display();
        cmd_reset();
    }
}


// Navigate to the beginning of the file 
void file_rewind(void) 
{
    fatfs_data.fresult = f_lseek(&fatfs_data.file, RESET_ZERO);
    (fatfs_data.fresult == FR_OK) ? feedback_display("\r\nNow at the beginning of the file.\r\n") : 
                                    fault_display();
    cmd_reset();
}


// Navigate to the end of the file 
void file_fast_fwd(void) 
{
    fatfs_data.fresult = f_lseek(&fatfs_data.file, f_size(&fatfs_data.file));
    (fatfs_data.fresult == FR_OK) ? feedback_display("\r\nNow at the end of the file.\r\n") : 
                                    fault_display();
    cmd_reset();
}


// Read from an open file using f_gets 
void file_gets(void)
{
    char file_data[DATA_BUFF_SIZE];

    // Read from the file 
    TCHAR *data_buff = f_gets(file_data, DATA_BUFF_SIZE, &fatfs_data.file);

    if (data_buff != NULL)
    {
        string_display("\r\nFile data: ", file_data);
    }
    else
    {
        if (f_eof(&fatfs_data.file) != 0)
        {
            feedback_display("\r\nEnd of file reached.");
        }
        else if (f_error(&fatfs_data.file) != 0)
        {
            feedback_display("\r\nHard file error occured.");
        }
        else
        {
            feedback_display("\r\nUnknown error.");
        }
    }

    cmd_reset();
}


// Write to an open file using f_puts 
void file_puts(void)
{
    if (fatfs_data.state_seq++ == 0)
    {
        feedback_display("\nFile data: ");
    }
    else
    {
        format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);

        // Format the input to end with a line break. This separates the provided data 
        // from "f_puts" into their own lines which also allows for each line to be 
        // retrieved individually when using "f_gets". 
        uint8_t buff_size = DATA_BUFF_SIZE + 2; 
        char line_of_data[buff_size];
        snprintf(line_of_data, buff_size, "%s\r\n", (char *)fatfs_data.data_in_buff);

        // Write a string to the file 
        int num_chars = f_puts(line_of_data, &fatfs_data.file);
        (num_chars >= 0) ? number_display("\r\nData written to file. Characters written: ", (DWORD)num_chars) : 
                           feedback_display("Write failed.");

        cmd_reset();
    }
}

//=======================================================================================


//=======================================================================================
// Helper functions 

// Select state based on user input 
void cmd_select(void)
{
    uint8_t index = CLEAR;
    format_input((char *)fatfs_data.data_in_buff, &fatfs_data.data_in_num, FORMAT_FILE_STRING);

    // Compare the input to the defined user commands 
    do
    {
        if (str_compare((char *)fatfs_data.data_in_buff, cmd_table[index].user_cmds, BYTE_0)) 
        {
            fatfs_data.state_func_ptr = cmd_table[index].fatfs_func_ptrs_t;
            fatfs_data.state_func_ptr();
            break; 
        }
    }
    while (++index < NUM_USER_CMDS);

    if (index >= NUM_USER_CMDS)
    {
        cmd_reset();
    }
}


// Return to default state at the end of the command dispatch 
void cmd_reset(void)
{
    fatfs_data.state_seq = CLEAR;
    fatfs_data.state_func_ptr = &cmd_select;
    feedback_display("\r\n>>> ");
}


// Display a formatting string with a string argument for the user to see 
void string_display(
    const char *string,
    const char *str_arg)
{
    const char str_format[] = "%s\r\n";
    snprintf(fatfs_data.data_out_buff, DATA_BUFF_SIZE, str_format, str_arg);
    feedback_display(string);
    feedback_display(fatfs_data.data_out_buff);
}


// Display a formatting string with a number argument for the user to see 
void number_display(
    const char *string,
    const DWORD num_arg)
{
    const char str_format[] = "%lu\r\n";
    snprintf(fatfs_data.data_out_buff, DATA_BUFF_SIZE, str_format, num_arg);
    feedback_display(string);
    feedback_display(fatfs_data.data_out_buff);
}


// Display the error code related to the FATFS operation 
void fault_display(void)
{
    snprintf(fatfs_data.data_out_buff,
             DATA_BUFF_SIZE,
             "\r\nProblem occurred --> FRESULT == %u\r\n",
             (uint8_t)fatfs_data.fresult);
    feedback_display(fatfs_data.data_out_buff);
}


// Display a string for the user to see 
void feedback_display(const char *string)
{
    uart_send_str(fatfs_data.uart, string);
}


// Format user input 
uint8_t format_input(
    char *buff, 
    DWORD *data, 
    format_user_input_t op)
{
    uint8_t result = FALSE; 

    if ((buff == NULL) || (data == NULL)) 
    {
        return result; 
    }

    if (op == FORMAT_FILE_STRING)
    {
        // Replace carriage return from input with a null character 
        for (uint8_t i = 0; i < CMD_SIZE; i++)
        {
            if (*buff == CR_CHAR)
            {
                *buff = NULL_CHAR; 
                break; 
            }
            buff++; 
        }

        result = TRUE; 
    }
    else if (op == FORMAT_FILE_MODE)
    {
        if (str_compare("0x", buff, BYTE_0))
        {
            uint8_t nibble; 
            *data = CLEAR; 
            
            // Check the character validity 
            for (uint8_t i = 2; i < 4; i++) 
            {
                nibble = buff[i]; 

                if ((nibble >= ZERO_CHAR) && (nibble <= NINE_CHAR))
                {
                    nibble -= NUM_TO_CHAR_OFFSET; 
                }
                else if ((nibble >= A_UP_CHAR) && (nibble <= F_UP_CHAR)) 
                {
                    nibble -= HEX_TO_LET_CHAR; 
                }
                else
                {
                    break;
                }

                *data |= (nibble << SHIFT_4*(3-i)); 

                if (i == 3)
                {
                    result = TRUE;
                }
            }
        }
    }
    else if (op == FORMAT_FILE_NUM)
    {
        char *buff_copy = buff; 

        while (*buff_copy != CR_CHAR) 
        {
            if (!((*buff_copy >= ZERO_CHAR) && (*buff_copy <= NINE_CHAR)))
            {
                break; 
            }
            buff_copy++; 
        }

        if (*buff_copy == CR_CHAR) 
        {
            *data = (DWORD)atoi(buff); 
            result = TRUE; 
        }
    }

    return result; 
}

//=======================================================================================
