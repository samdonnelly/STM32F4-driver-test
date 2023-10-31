# STM32F4 Test 

This repository contains test code written for the STM32F411RE chip. It's intended to test sandbox application code as well as drivers and software tools from the <a href="https://github.com/samdonnelly/STM32F4-driver-library">STM32F4-driver-library</a>. The default branch 'template' is meant as a starting point for setting up any new project that uses the STM32F411RE. 

The test code is broken into the following sections: 

### Device Configuration 

Configuration files needed to set up devices on startup. Some devices need to be configured using specific instructions/messages and these instructions change based on the application so they're not part of the device driver. 

### Driver Test 

Test code for the device and peripheral drivers found in the <a href="https://github.com/samdonnelly/STM32F4-driver-library">STM32F4-driver-library</a>. This focuses on testing the functionality of the driver as opposed to using the driver in a specific application. 

### Application Test 

Sample application code. This code is used to implement and test functionality that is less focused on the device and peripheral drivers. Some of this functionality could include calculations, algorithms, language specific features, system architecture or methods for handling data. The drivers from the <a href="https://github.com/samdonnelly/STM32F4-driver-library">STM32F4-driver-library</a> are applied as needed. 
