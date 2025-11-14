# STM32 Minimal CMake project for C/C++ projects
cmake_minimum_required(VERSION 3.12)

###############################################################################

include(./build_tools/arm-none-eabi-gcc.cmake)

# A "SystemConfig.txt" file must be created in the build_tools folder of this 
# project and within it the RTOS_ENABLE must be defined as "set(RTOS_ENABLE X)" 
# where "X" is either 0 or 1 depending on how you configure your system. A 1 
# will include FreeRTOS in the project. 
# "SystemConfig.txt" is not tracked so users can configure their system without 
# making changes to the project. The value of this variable must match the 
# value of "FREERTOS_ENABLE" from the "hardware_config.h" file 
include(${CMAKE_CURRENT_SOURCE_DIR}/build_tools/SystemConfig.txt)

# Set project name and source code folder location
project(STM32F4-driver-test)

option(DUMP_ASM "Create full assembly of final executable" OFF)

# Set microcontroller information
set(MCU_FAMILY STM32F4xx)
set(MCU_MODEL STM32F411xE)
set(CPU_PARAMETERS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard)

set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/build_tools/startup_stm32f411xe.s)
set(MCU_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/build_tools/STM32F411RETx_FLASH.ld)

###############################################################################
set(EXECUTABLE ${CMAKE_PROJECT_NAME})

enable_language(C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

###############################################################################

###############################################################################
# Headers 

# STM code 
set(STM32CUBEMX_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/fatfs
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Middlewares/Third_Party/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/CMSIS/Device/ST/STM32F4xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy)

# Project and driver code 
set(PROJECT_INCLUDE_DIRECTORIES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_data
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/data_handling_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/external_input_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/pose_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/rtos_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/actuator_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/display_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/gnss_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/imu_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/memory_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/wireless_test
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/peripheral_tests
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/actuators
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/displays
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/gnss
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/imu
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/memory
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/wireless
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/peripherals
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/other
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/tools)

# External library code 
set(LIBRARY_INCLUDE_DIRECTORIES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../mavlink/c_library_v2
    ${CMAKE_CURRENT_SOURCE_DIR}/../mavlink/c_library_v2/standard)

###############################################################################

###############################################################################
# Sources 

# STM code 
if (RTOS_ENABLE EQUAL 1) 
file(GLOB_RECURSE STM32CUBEMX_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/fatfs/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Middlewares/Third_Party/FreeRTOS/Source/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c)
else()
file(GLOB_RECURSE STM32CUBEMX_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/fatfs/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/stmcode/Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c)
endif() 

# Project and driver code 
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interface/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_data/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_data/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/data_handling_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/data_handling_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/external_input_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/external_input_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/pose_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/pose_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/rtos_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/application_tests/rtos_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/actuator_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/actuator_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/display_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/display_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/gnss_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/gnss_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/imu_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/imu_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/memory_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/memory_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/wireless_test/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/device_tests/wireless_test/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/peripheral_tests/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/peripheral_tests/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/actuators/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/actuators/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/displays/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/displays/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/gnss/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/gnss/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/imu/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/imu/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/memory/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/memory/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/wireless/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/devices/wireless/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/peripherals/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/peripherals/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/other/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/stm32f4/other/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/tools/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4-driver-library/tools/*.cpp)

###############################################################################

# Executable files 
add_executable(${EXECUTABLE}
    ${STM32CUBEMX_SOURCES} 
    ${PROJECT_SOURCES}
    ${STARTUP_SCRIPT})

# Embedded macros (defines) 
target_compile_definitions(${EXECUTABLE} PRIVATE
    #$<$<CONFIG:Debug>:DEBUG>
    ${MCU_MODEL}
    USE_HAL_DRIVER)

# Add header directories (***AFTER add_executable) 
target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
    ${STM32CUBEMX_INCLUDE_DIRECTORIES})
target_include_directories(${EXECUTABLE} PRIVATE
    ${PROJECT_INCLUDE_DIRECTORIES})
target_include_directories(${EXECUTABLE} PRIVATE
    ${LIBRARY_INCLUDE_DIRECTORIES})

###############################################################################

# Compiler and linker options 
target_compile_options(${EXECUTABLE} PRIVATE
    ${CPU_PARAMETERS}
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wdouble-promotion
        -Wformat=2 -Wformat-truncation
        -Wundef
        -fno-common
        -Wno-unused-parameter
        #######################################
        # Added by me 
        # Due to mavlink library inclusion: 
        -Wno-pedantic 
        -Wno-address-of-packed-member
        #######################################
        $<$<COMPILE_LANGUAGE:CXX>:
            # -Wconversion # STM libraries!
            -Wno-volatile
            # -Wold-style-cast
            # -Wuseless-cast
            -Wsuggest-override>
        $<$<CONFIG:Debug>:-Og -g3 -ggdb>
        $<$<CONFIG:Release>:-Og -g0>)

    target_link_options(${EXECUTABLE} PRIVATE
        -T${MCU_LINKER_SCRIPT}
        ${CPU_PARAMETERS}
        -Wl,-Map=${CMAKE_PROJECT_NAME}.map
        --specs=nosys.specs
        -Wl,--start-group
        -lc
        -lm
        -lstdc++
        -Wl,--end-group
        -Wl,--print-memory-usage)

# The last command can take a couple of seconds on larger project, usefull for debugging
add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${EXECUTABLE}>
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${EXECUTABLE}> ${EXECUTABLE}.bin)

if (${DUMP_ASM})
    add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
        COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:${EXECUTABLE}> > ${EXECUTABLE}.s)
endif()
