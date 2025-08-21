## <b>CRYP_SAES_EncDec Example Description</b>
  
How to use the Secure AES co-processor (SAES) peripheral to do data encryption/decryption with DHUK, BHK and DHUK xor BHK

There will be a test menu for test cases using AES GCM, ECB and CBC algorithms
============== SAES Test Menu =================

  Test AES GCM   ------------------ 1

  Test AES ECB  ------------------- 2

  Test AES CBC  ------------------- 3

Enter 1, 2 or 3 will show submenu for each algo. e.g. Select '1' will show GCM test submenu
============== SAES GCM Test Menu =================

  Write key to BHK           ------------------- b

  Lock BHK to avoid SW read  ------------------- l

  Read BHK from BKP register ------------------- r

  ================================================

  Key select normal (SW key) ------------------- 1

  Key select DHUK            ------------------- 2

  Key select BHK             ------------------- 3

  Key select DHUK xor BHK    ------------------- 4

  Exit                       ------------------- x

  To test the use case of AES encryption/decryption with pure SW key written from application to SAES key register, enter '1'
  To test the use case of AES encryption/decryption with DHUK, enter '2'
  To test the use case of AES encryption/decryption with BHK, enter '3'
  To test the use case of AES encryption/decryption with BHK xor DHUK, enter '4'

  For test case with BHK involved, you must first enter 'b' to write the keys to BHK (BKP registers 0~8)
  After that you can read back the value from BHK BKP registers by typing 'r'
  Typing 'l' will lock BHK registers, in such case typing 'r' will read back 0s only. 
  And after that if you want to write new keys (e.g. for the purpose of testing BHK as key for a different algo) you have to 
  power off/on the board first and run the program again

main.c will only initialize RNG and RTC peripherals and the initialization of SAES will be done in ecb_cbc.c and gcm.c respectively
Because SAES initialization will be different for different use cases

RNG is initialized globally because it is required for SAES
RTC is initialized globally becasue BHK requires key data to be written to BKP registers0~8 and RTC need to be initialized for that


#### <b>Notes</b>
1. Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.

2. The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

### <b>Keywords</b>

System, Security, SAES, AES,  TrustZone disabled

### <b>Directory contents</b>

  - CRYP/CRYP_SAES_EncDec/FSBL/Src/main.c                         Main program
  - CRYP/CRYP_SAES_EncDec/FSBL/Src/ecb_cbc.c                      ECB and CBC enc/dec test program
  - CRYP/CRYP_SAES_EncDec/FSBL/Src/gdm.c                          GCM enc/dec test program
  - CRYP/CRYP_SAES_EncDec/FSBL/Src/system_stm32n6xxc              STM32N6xx system clock configuration file
  - CRYP/CRYP_SAES_EncDec/FSBL/Src/stm32n6xx_it.c                 Interrupt handlers
  - CRYP/CRYP_SAES_EncDec/FSBL/Src/stm32n6xx_hal_msp.c            HAL MSP module
  - CRYP/CRYP_SAES_EncDec/FSBL/Inc/main.h                         Main program header file
  - CRYP/CRYP_SAES_EncDec/FSBL/Inc/stm32n6xx_nucleo_conf.h        BSP Configuration file
  - CRYP/CRYP_SAES_EncDec/FSBL/Inc/stm32n6xx_hal_conf.h           HAL Configuration file
  - CRYP/CRYP_SAES_EncDec/FSBL/Inc/stm32n6xx_it.h                 Interrupt handlers header file

### <b>Hardware and Software environment</b>

  - This example runs on STM32N657X0HxQ devices.

  - This example has been tested with STMicroelectronics NUCLEO-N657X0-Q
    board and can be easily tailored to any other supported device
    and development board.

  - **EWARM** : To monitor a variable in the live watch window, you must proceed as follow :
    - Start a debugging session.
    - Open the View > Images.
    - Double-click to deselect the second instance of project.out. 

  - **MDK-ARM** : it is not possible to monitor a variable in the live watch window    

### <b>How to use it ?</b> 

In order to make the program work, you must do the following :

 - Set the boot mode in development mode (BOOT1 switch position is 2-3, BOOT0 switch position doesn't matter).
 - Open your preferred toolchain
 - Rebuild all files and load your image into target memory. Code can be executed in this mode for debugging purposes.

 Next, this program can be run in boot from flash mode. This is done by following the instructions below:

 - Resort to CubeProgrammer to add a header to the generated binary Project.bin with the following command
   - *STM32_SigningTool_CLI.exe -bin Project.bin -nk -of 0x80000000 -t fsbl -o Project-trusted.bin -hv 2.3 -dump Project-trusted.bin*
   - The resulting binary is Project-trusted.bin.
 - Next, in resorting again to CubeProgrammer, load the binary and its header (Project-trusted.bin) in the board external Flash at address 0x7000'0000.
 - Set the boot mode in boot from external Flash (BOOT0 switch position is 1-2 and BOOT1 switch position is 1-2).
 - Press the reset button. The code then executes in boot from external Flash mode.
