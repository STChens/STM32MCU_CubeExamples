/**
  ******************************************************************************
  * @file    AppliNonSecure/Src/main.c
  * @author  GPM Application Team
  * @brief   Non-secure main program.
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT 2024 STMicroelectronics
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "secure_nsc.h"

#include "common.h"
#include "com.h"
#include "fw_update_app.h"
#include "ns_data.h"
#include "appli_flash_layout.h"

/** @addtogroup STM32N6xx_HAL_Template
  * @{
  */

/** @addtogroup HAL
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Enable print of boot time (obtained through DWT).
   DWT usage requires product state is not closed/locked.
   OEMxRoT logs must be disabled for relevant boot time. */
#define PRINT_BOOT_TIME

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t *pUserAppId;
const uint8_t UserAppId = 'A';
static uint64_t time;
static uint32_t end;

static Fault_Info_t fault_info;
static IAC_Info_t iac_info;

/* Private function prototypes -----------------------------------------------*/
void FW_APP_PrintMainMenu(void);
void FW_APP_Run(void);

void SecureFault_Callback(void);
void SecureError_Callback(void);

#if defined(__ICCARM__)
/* New definition from EWARM V9, compatible with EWARM8 */
int iar_fputc(int ch);
#define PUTCHAR_PROTOTYPE int iar_fputc(int ch)
#elif defined ( __CC_ARM ) || defined(__ARMCC_VERSION)
/* ARM Compiler 5/6*/
int io_putchar(int ch);
#define PUTCHAR_PROTOTYPE int io_putchar(int ch)
#elif defined(__GNUC__)
#define PUTCHAR_PROTOTYPE int32_t uart_putc(int32_t ch)
#endif /* __ICCARM__ */

PUTCHAR_PROTOTYPE
{
  COM_Transmit((uint8_t*)&ch, 1, TX_TIMEOUT);
  return ch;
}

/* Redirects printf to DRIVER_STDIO in case of ARMCLANG*/
#if defined(__ARMCC_VERSION)
FILE __stdout;

/* __ARMCC_VERSION is only defined starting from Arm compiler version 6 */
int fputc(int ch, FILE *f)
{
  /* Send byte to USART */
  io_putchar(ch);

  /* Return character written */
  return ch;
}
#elif defined(__GNUC__)
/* Redirects printf to DRIVER_STDIO in case of GNUARM */
int _write(int fd, char *str, int len)
{
  int i;

  for (i = 0; i < len; i++)
  {
    /* Send byte to USART */
    uart_putc(str[i]);
  }

  /* Return the number of characters written */
  return len;
}
#elif defined(__ICCARM__)
size_t __write(int file, unsigned char const *ptr, size_t len)
{
  size_t idx;
  unsigned char const *pdata = ptr;

  for (idx = 0; idx < len; idx++)
  {
    iar_fputc((int)*pdata);
    pdata++;
  }
  return len;
}
#endif /*  __GNUC__ */

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* Get boot cycles */
  end = DWT->CYCCNT;

  /* Reset of all peripherals, Initializes the Flash interface and the systick. */
  HAL_Init();

  /* Get Boot Time */
  time = ((uint64_t)(end) * 1000000U / SystemCoreClock);

  /* Register SecureFault callback defined in non-secure and to be called by secure handler */
  SECURE_RegisterCallback(SECURE_FAULT_CB_ID, (void *)SecureFault_Callback, &fault_info);

  /* Register SecureError callback defined in non-secure and to be called by secure handler */
  SECURE_RegisterCallback(IAC_ERROR_CB_ID, (void *)SecureError_Callback, &iac_info);

  /*******************************************************************************
  *                          Common Configuration Routines                       *
  *******************************************************************************/

  pUserAppId = (uint8_t *)&UserAppId;

  COM_Init();

#ifdef PRINT_BOOT_TIME
  printf("\r\nBoot time : %u ms at %u MHz", (unsigned int)(time/1000), (unsigned int)(SystemCoreClock/1000000U));
  printf("\r\n");
#endif
  printf("\r\n======================================================================");
  printf("\r\n=              (C) COPYRIGHT 2024 STMicroelectronics                 =");
  printf("\r\n=                                                                    =");
  printf("\r\n=                          User App #%c                               =", *pUserAppId);
  printf("\r\n======================================================================");
  printf("\r\n\r\n");

  /* User App firmware runs*/
  FW_APP_Run();
}

/**
  * @brief  Display the TEST Main Menu choices on HyperTerminal
  * @param  None.
  * @retval None.
  */
void FW_APP_PrintMainMenu(void)
{
  printf("\r\n=================== Main Menu ============================\r\n\n");
  printf("  Enable non-secure debug ------------------------------- d\r\n\n");
  printf("  Get debug enable state  ------------------------------- s\r\n\n");
#if (NS_DATA_IMAGE_NUMBER == 1)
  printf("  Non-Secure Data --------------------------------------- 1\r\n\n");
#endif /* NS_DATA_IMAGE_NUMBER == 1 */
#if (DOWNLOAD_MENU == 1)
  printf("  New Fw Image ------------------------------------------ 2\r\n\n");
#endif /* DOWNLOAD_MENU == 1 */
  printf("  Selection :\r\n\n");
}

/**
  * @brief  Display the TEST Main Menu choices on HyperTerminal
  * @param  None.
  * @retval None.
  */
void FW_APP_Run(void)
{
  uint8_t key = 0U;

  /*##1- Print Main Menu message*/
  FW_APP_PrintMainMenu();

  while (1U)
  {
    /* Clean the input path */
    COM_Flush();

    /* Receive key */
    if (COM_Receive(&key, 1U, RX_TIMEOUT) == HAL_OK)
    {
      switch (key)
      {
        case 'd' :
          printf("  Call NSC API to enable debug for Non-secure part \r\n\n");
          SECURE_EnableNSDebug();
          break;

        case 's' :
          printf("  Call NSC API to get debug state \r\n\n");
          {
            uint32_t apunlock;            
            BSEC_DebugCfgTypeDef dbgCfg;
            uint32_t current_hdpl;
            
            SECURE_GetDebugState(&apunlock, &dbgCfg, &current_hdpl);
            printf("\tCurrent HDP Level: %08x [%s]\r\n", current_hdpl, \
                                            (current_hdpl == 0xB4) ? "HDPL0": \
                                            ((current_hdpl == 0x51) ? "HDPL1": \
                                              ((current_hdpl == 0x8A) ? "HDPL2": \
                                                    ((current_hdpl == 0x6F) ? "HDPL3": "invalid"))));            
            printf("\tAP UNLOCK state   : %08x [%s]\r\n", apunlock, (apunlock == HAL_BSEC_DEBUG_UNLOCKED) ? "Unlocked": "Locked");
            printf("\tDebug unlock state: %08x [%s]\r\n", dbgCfg.NonSec_Dbg_Auth, (dbgCfg.NonSec_Dbg_Auth == HAL_BSEC_NONSEC_DBG_AUTH) ? "Enabled": "Disabled" );
            printf("\tSecure Debug auth state: %08x [%s]\r\n", dbgCfg.Sec_Dbg_Auth, (dbgCfg.Sec_Dbg_Auth == HAL_BSEC_SEC_DBG_AUTH) ? "Enabled": "Disabled");
            printf("\tDebug allowed HDP Level: %08x [%s]\r\n", dbgCfg.HDPL_Open_Dbg, \
							  (dbgCfg.HDPL_Open_Dbg == HAL_BSEC_OPEN_DBG_LEVEL_0) ? "HDPL0": \
							  ((dbgCfg.HDPL_Open_Dbg == HAL_BSEC_OPEN_DBG_LEVEL_1) ? "HDPL1": \
								((dbgCfg.HDPL_Open_Dbg == HAL_BSEC_OPEN_DBG_LEVEL_2) ? "HDPL2": \
								  ((dbgCfg.HDPL_Open_Dbg == HAL_BSEC_OPEN_DBG_LEVEL_3) ? "HDPL3": "invalid"))));
          }
          break;        
#if (NS_DATA_IMAGE_NUMBER == 1)
        case '1' :
          NS_DATA_Run();
          break;
#endif /* NS_DATA_IMAGE_NUMBER == 1 */
#if (DOWNLOAD_MENU == 1)
        case '2' :
          FW_UPDATE_Run();
          break;
#endif /* DOWNLOAD_MENU == 1 */
        default:
          printf("Invalid Number !\r");
          break;
      }

      /* Print Main Menu message */
      FW_APP_PrintMainMenu();
    }
  }
}

/**
  * @brief  Callback called by secure code following a secure fault interrupt
  * @note   This callback is called by secure code thanks to the registration
  *         done by the non-secure application with non-secure callable API
  *         SECURE_RegisterCallback(SECURE_FAULT_CB_ID, (void *)SecureFault_Callback);
  * @retval None
  */
void SecureFault_Callback(void)
{
  /* Go to infinite loop when Secure fault generated by IDAU/SAU check */
  /* because of illegal access */
  printf("%s:%d\r\n", __FUNCTION__, __LINE__);
  printf("============================================\r\n");
  printf("CFSR: %08x\r\n", fault_info.cfsr);
  printf("CFSR_NS: %08x\r\n", fault_info.cfsr_ns);
  printf("BFAR: %08x\r\n", fault_info.bfar);
  printf("MMFAR: %08x\r\n", fault_info.mmfar);
  printf("SFSR: %08x\r\n", fault_info.sfsr);
  printf("SFAR: %08x\r\n", fault_info.sfar);
  printf("TAMP_SR: %08x\r\n", fault_info.tamp_sr);
  
  while (1)
  {
  }
}

/**
  * @brief  Callback called by secure code following a IAC secure interrupt (IAC_IRQn)
  * @note   This callback is called by secure code thanks to the registration
  *         done by the non-secure application with non-secure callable API
  *         SECURE_RegisterCallback(IAC_ERROR_CB_ID, (void *)SecureError_Callback);
  * @retval None
  */
void SecureError_Callback(void)
{
  /* Go to infinite loop when Secure error generated by RIF IAC check */
  /* because of illegal access */
  printf("%s:%d\r\n", __FUNCTION__, __LINE__);
  printf("============================================\r\n");
  printf("IAC_IISR0: %08x\r\n", iac_info.iac_iisr[0]);
  printf("IAC_IISR1: %08x\r\n", iac_info.iac_iisr[1]);
  printf("IAC_IISR2: %08x\r\n", iac_info.iac_iisr[2]);
  printf("IAC_IISR3: %08x\r\n", iac_info.iac_iisr[3]);
  printf("IAC_IISR4: %08x\r\n", iac_info.iac_iisr[4]);
  printf("IAC_IISR5: %08x\r\n", iac_info.iac_iisr[5]);
    
  while (1)
  {
  }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(file);
  UNUSED(line);

  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */
