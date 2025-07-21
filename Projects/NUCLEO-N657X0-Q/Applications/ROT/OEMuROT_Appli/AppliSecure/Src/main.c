/**
  ******************************************************************************
  * @file    AppliSecure/Src/main.c
  * @author  GPM Application Team
  * @brief   Secure main program.
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
#include "stm32n6xx_it.h"
#include "low_level_ext_flash.h"
#include "appli_flash_layout.h"

#include "stm32n6xx_hal.h"

#include <stdio.h>

/** @addtogroup STM32N6xx_HAL_Template
  * @{
  */
/** @addtogroup HAL
  * @{
  */
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/* Non-secure Vector table to jump to                                         */
/* Caution: address must correspond to non-secure address as it is mapped in  */
/*          the non-secure vector table                                       */

#define VTOR_TABLE_NS_START_ADDR 0x24064400 /* This define is updated automatically from OEMuROT_Boot project */

/* Private macro -------------------------------------------------------------*/
#define BSEC_NB_FUSES                376U

/* Private variables ---------------------------------------------------------*/
extern ARM_DRIVER_FLASH Driver_EXT_FLASH0;

/* Private function prototypes -----------------------------------------------*/

/* Global variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief System Clock Configuration
  * @retval None
  */

#define COM_INSTANCE                           USART1
#define COM_CLK_ENABLE()                       __HAL_RCC_USART1_CLK_ENABLE()
#define COM_CLK_DISABLE()                      __HAL_RCC_USART1_CLK_DISABLE()

#define COM_TX_GPIO_PORT                       GPIOE
#define COM_TX_GPIO_CLK_ENABLE()               __HAL_RCC_GPIOE_CLK_ENABLE()
#define COM_TX_PIN                             GPIO_PIN_5
#define COM_TX_AF                              GPIO_AF7_USART1

#define COM_RX_GPIO_PORT                       GPIOE
#define COM_RX_GPIO_CLK_ENABLE()               __HAL_RCC_GPIOE_CLK_ENABLE()
#define COM_RX_PIN                             GPIO_PIN_6
#define COM_RX_AF                              GPIO_AF7_USART1

static UART_HandleTypeDef  uart_device;
static int32_t USART0_Initialize(void)
{

  GPIO_InitTypeDef GPIO_Init;
  /* Configure COM Tx as alternate function */
  COM_TX_GPIO_CLK_ENABLE();
  COM_RX_GPIO_CLK_ENABLE();
  COM_CLK_ENABLE();
  GPIO_Init.Pin       = COM_TX_PIN;
  GPIO_Init.Mode      = GPIO_MODE_AF_PP;
  GPIO_Init.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_Init.Pull      = GPIO_PULLUP;
  GPIO_Init.Alternate = COM_TX_AF;
  HAL_GPIO_Init(COM_TX_GPIO_PORT, &GPIO_Init);

  /* Configure COM Rx as alternate function */
  GPIO_Init.Pin       = COM_RX_PIN;
  GPIO_Init.Alternate = COM_RX_AF;
  HAL_GPIO_Init(COM_RX_GPIO_PORT, &GPIO_Init);

  uart_device.Instance = COM_INSTANCE;
  uart_device.Init.BaudRate       = 115200;
  uart_device.Init.WordLength     = UART_WORDLENGTH_8B;
  uart_device.Init.StopBits       = UART_STOPBITS_1;
  uart_device.Init.Parity         = UART_PARITY_NONE;
  uart_device.Init.Mode           = UART_MODE_TX_RX;
  uart_device.Init.HwFlowCtl      = UART_HWCONTROL_NONE;
  uart_device.Init.OverSampling   = UART_OVERSAMPLING_8;
  uart_device.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  uart_device.Init.ClockPrescaler = UART_PRESCALER_DIV1;

  /* Initialize COM */
  if (HAL_UART_Init(&uart_device) != HAL_OK)
  {
    return ARM_DRIVER_ERROR;
  }
  return ARM_DRIVER_OK;
}

/* USER CODE BEGIN 4 */
static void stdio_output_string(const unsigned char *str, uint32_t len)
{
	if ( str != NULL && len != 0 )
		HAL_UART_Transmit(&uart_device, (uint8_t *) str, len,  1000);
}

/* Redirects printf to DRIVER_STDIO in case of ARMCLANG*/
#if defined(__ARMCC_VERSION)
/* Struct FILE is implemented in stdio.h. Used to redirect printf to
 * STDIO_DRIVER
 */
FILE __stdout;
/* __ARMCC_VERSION is only defined starting from Arm compiler version 6 */
int fputc(int ch, FILE *f)
{
    (void)f;

    /* Send byte to USART */
    (void)stdio_output_string((const unsigned char *)&ch, 1);
    
    /* Return character written */
    return ch;
}
#elif defined(__GNUC__)
/* Redirects printf to STDIO_DRIVER in case of GNUARM */
int _write(int fd, char *str, int len)
{
    (void)fd;

    /* Send string and return the number of characters written */
    stdio_output_string((const unsigned char *)str, (uint32_t)len);
    return len;
}
#elif defined(__ICCARM__)
int putchar(int ch)
{
    /* Send byte to USART */
    (void)stdio_output_string((const unsigned char *)&ch, 1);
    /* Return character written */
    return ch;
}
#endif




/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  
  while (1);
  /* USER CODE END Error_Handler_Debug */
}

/**
  * @brief   This function opens the s and/or ns debug for the current HDPL.
  * @param  ns_only: 1 to open debug for non-secure only, 0 to open full debug
  * @retval None
  */
void open_full_debug(int ns_only) {
  BSEC_HandleTypeDef sBsecHandler = {.Instance = BSEC};
  uint32_t hdpl;
  BSEC_DebugCfgTypeDef dbgCfg = {0};
  
  HAL_BSEC_UnlockDebug(&sBsecHandler);

  HAL_BSEC_GetHDPLValue(&sBsecHandler, &hdpl);
  
  dbgCfg.HDPL_Open_Dbg = hdpl << 16;
  dbgCfg.Sec_Dbg_Auth = ns_only == 1 ? HAL_BSEC_SEC_DBG_UNAUTH : HAL_BSEC_SEC_DBG_AUTH;
  dbgCfg.NonSec_Dbg_Auth = HAL_BSEC_NONSEC_DBG_AUTH;
  HAL_BSEC_ConfigDebug(&sBsecHandler, &dbgCfg);
}

/**
  * @brief  Function to get the current debug port state configured in BSEC 
  * @param  apunlock: pointer to the buffer to hold output AP UNLOCK state
  * @param  pDbgCfg: pointer to the buffer to hold the output BSEC_DebugCfgTypeDef data
  * @param  hdpl: pointer to hold the output HDPL info
  * @retval None
  */
void get_dbg_state(uint32_t *apunlock, BSEC_DebugCfgTypeDef *pDbgCfg, uint32_t *hdpl)
{
  BSEC_HandleTypeDef sBsecHandler = {.Instance = BSEC};
  uint32_t ap, hdp;
	BSEC_DebugCfgTypeDef dbg;

	HAL_BSEC_GetDebugLockState(&sBsecHandler, &ap);
	HAL_BSEC_GetDebugConfig(&sBsecHandler, &dbg);
	HAL_BSEC_GetHDPLValue(&sBsecHandler, &hdp);
	
  if ( apunlock != NULL )
  {
	  *apunlock = ap;
  }
  if ( pDbgCfg != NULL )
  {
	  *pDbgCfg = dbg;
  }
  if ( hdpl != NULL )
  {
	  *hdpl = hdp;
  }
}


/**
  * @brief  Function to read otp shadow registers
  * @param  wordIdx: start index of OTP word
  * @param  nbWords: number of words to read
  * @param  outBuf: pointer to the output buffer 
  * @retval None
  */
void bsec_otp_read(uint32_t wordIdx, int nbWords, uint32_t *outBuf)
{
  HAL_StatusTypeDef status;
  uint32_t *pFuseData = outBuf;
  uint32_t FuseId = wordIdx;
  BSEC_HandleTypeDef sBsecHandler = {.Instance = BSEC};
  int i;
  
  if ((outBuf != NULL) && ((wordIdx + nbWords) < BSEC_NB_FUSES))  
  
  for ( i = 0; i < nbWords; i++, pFuseData++, FuseId++)
  {
    status = HAL_BSEC_OTP_ReadShadow(&sBsecHandler, FuseId, pFuseData);    
  }
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

int main(void)
{
  funcptr_NS NonSecure_ResetHandler;

  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();

  /* Enable BusFault and SecureFault handlers (HardFault is default) */
  SCB->SHCSR |= (SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_SECUREFAULTENA_Msk);

  /* Reset of all peripherals, Initializes the Flash interface and the systick. */
  HAL_Init();
  
  //SystemClock_Config();
  
  
  /* All IOs are by default allocated to secure */
  /* Release them all to non-secure */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPION_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();
  __HAL_RCC_GPIOP_CLK_ENABLE();
  __HAL_RCC_GPIOQ_CLK_ENABLE();

  HAL_GPIO_ConfigPinAttributes(GPIOA, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOB, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOC, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOD, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOE, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOF, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOG, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOH, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPION, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOO, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOP, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  HAL_GPIO_ConfigPinAttributes(GPIOQ, GPIO_PIN_ALL, GPIO_PIN_NSEC);
  
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOF_CLK_DISABLE();
  __HAL_RCC_GPIOG_CLK_DISABLE();
  __HAL_RCC_GPIOH_CLK_DISABLE();
  __HAL_RCC_GPION_CLK_DISABLE();
  __HAL_RCC_GPIOO_CLK_DISABLE();
  __HAL_RCC_GPIOP_CLK_DISABLE();
  __HAL_RCC_GPIOQ_CLK_DISABLE();

  /* Set USART1 as configurable by non-secure */
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_USART1, RIF_ATTRIBUTE_NSEC);
  
  USART0_Initialize();
  
  printf("\r\n===> In App S: COM Init done.\r\n");
  
#if (DOWNLOAD_MENU == 1)
  Driver_EXT_FLASH0.Initialize(NULL);
#endif /* DOWNLOAD_MENU == 1 */

  /*************** Setup and jump to non-secure *******************************/

  /* Set non-secure vector table location */
  SCB_NS->VTOR = VTOR_TABLE_NS_START_ADDR;

  /* Set non-secure main stack (MSP_NS) */
  __TZ_set_MSP_NS((*(uint32_t *)VTOR_TABLE_NS_START_ADDR));

  /* Get non-secure reset handler */
  NonSecure_ResetHandler = (funcptr_NS)(*((uint32_t *)((VTOR_TABLE_NS_START_ADDR) + 4U)));

  /* Start non-secure state software application */
  NonSecure_ResetHandler();

  /* Non-secure software does not return, this code is not executed */
  while (1) {
    __NOP();
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
