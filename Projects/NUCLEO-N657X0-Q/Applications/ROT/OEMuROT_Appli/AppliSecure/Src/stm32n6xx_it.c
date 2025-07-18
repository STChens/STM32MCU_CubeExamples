/**
  ******************************************************************************
  * @file    AppliSecure/Src/stm32n6xx_it.c
  * @author  GPM Application Team
  * @brief   Secure Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * COPYRIGHT(c) 2024 STMicroelectronics
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
#include "secure_nsc.h"
#include "appli_flash_layout.h"

extern funcptr_NS pSecureFault_Callback;
extern funcptr_NS pSecureError_Callback;
extern funcptr_NS pSecureIT_Callback;
extern void *pSecureFault_pData;
extern void *pSecureError_pData;

#if defined(__ICCARM__)
typedef void (CMSE_NS_CALL *SecureFault_Callback)(void);
typedef void (CMSE_NS_CALL *SecureError_Callback)(void);
typedef void (CMSE_NS_CALL *SecureIT_Callback)(IRQn_Type IrqLine);
#else
typedef void CMSE_NS_CALL (*SecureFault_Callback)(void);
typedef void CMSE_NS_CALL (*SecureError_Callback)(void);
typedef void CMSE_NS_CALL (*SecureIT_Callback)(IRQn_Type IrqLine);
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#if (NS_DATA_IMAGE_NUMBER == 1)
#define LOG_RAM_BASE_ADD NS_DATA_ADDRESS
#endif
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M55 Processor Exceptions Handlers                         */
/******************************************************************************/


/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

void save_log(uint32_t lr, uint32_t msp, uint32_t msp_ns)
{
#if (NS_DATA_IMAGE_NUMBER == 1)
  uint32_t *pfootprint = (uint32_t*)(LOG_RAM_BASE_ADD+0x100);
  *pfootprint = (uint32_t)pfootprint;
  *pfootprint++ = SCB->CFSR;
  *pfootprint++ = SCB_NS->CFSR;
  *pfootprint++ = SCB->SFSR;
  *pfootprint++ = SCB->SFAR;
  *pfootprint++ = SCB->BFAR;
  *pfootprint++ = SCB->MMFAR;
  *pfootprint++ = lr;
  if ( (lr & 0xa0) == 0xa0) // NS triggered the fault
  {
	  *pfootprint++ = msp_ns;
	  *pfootprint++ = *(uint32_t *)msp_ns;
	  *pfootprint++ = *(uint32_t *)(msp_ns+4); // R0
	  *pfootprint++ = *(uint32_t *)(msp_ns+8); // R1
	  *pfootprint++ = *(uint32_t *)(msp_ns+12); // R2
	  *pfootprint++ = *(uint32_t *)(msp_ns+16); // R3
	  *pfootprint++ = *(uint32_t *)(msp_ns+20); // R12
	  *pfootprint++ = *(uint32_t *)(msp_ns+20); // LR
	  *pfootprint++ = *(uint32_t *)(msp_ns+20); // PC
	  *pfootprint++ = *(uint32_t *)(msp_ns+20); // xPSR
  }
  else
  {
	  *pfootprint++ = msp;
	  *pfootprint++ = *(uint32_t *)msp_ns;
	  *pfootprint++ = *(uint32_t *)(msp+4); // R0
	  *pfootprint++ = *(uint32_t *)(msp+8); // R1
	  *pfootprint++ = *(uint32_t *)(msp+12); // R2
	  *pfootprint++ = *(uint32_t *)(msp+16); // R3
	  *pfootprint++ = *(uint32_t *)(msp+20); // R12
	  *pfootprint++ = *(uint32_t *)(msp+20); // LR
	  *pfootprint++ = *(uint32_t *)(msp+20); // PC
	  *pfootprint++ = *(uint32_t *)(msp+20); // xPSR
  }
  *pfootprint++ = 0;
  *pfootprint++ = 0;
  *pfootprint++ = 0;
  *pfootprint++ = 0;
  *pfootprint++ = 0;
  *pfootprint++ = 0;
  *pfootprint++ = 0;
  *pfootprint++ = 0;
#endif
}

funcptr_NS callback_NS; // non-secure callback function pointer

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  __asm volatile (
      "MOV R0, LR\n"

      "MRS R1, MSP\n"

      "MRS R2, MSP_NS\n"

      // 跳转到C函数进行后续处理
      "B save_log\n"
  );


  if(pSecureFault_Callback != (funcptr_NS)NULL)
  {
    if ( pSecureFault_pData != NULL )
    {
      Fault_Info_t *info = (Fault_Info_t *)pSecureFault_pData;
      info->cfsr = SCB->CFSR;
      info->cfsr_ns = SCB_NS->CFSR;
      info->mmfar = SCB->MMFAR;
      info->bfar = SCB->BFAR;
      info->sfsr = SAU->SFSR;
      info->sfar = SAU->SFSR;
      info->tamp_sr = TAMP->SR;
    }
   /* return function pointer with cleared LSB */
   callback_NS = (funcptr_NS)cmse_nsfptr_create(pSecureFault_Callback);

   callback_NS();
  }

  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  __asm volatile (
	  "MOV R0, LR\n"

	  "MRS R1, MSP\n"

	  // 读取MSP_NS寄存器到R2（通过VTOR基址+0x00获取MSP_NS）
	  "LDR R2, =0xE000ED00\n"  // VTOR基址（向量表起始地址）
	  "LDR R2, [R2]\n"         // 读取MSP_NS的值

	  // 跳转到C函数进行后续处理
	  "B save_log\n"
  );
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  __asm volatile (
	  "MOV R0, LR\n"

	  "MRS R1, MSP\n"

	  // 读取MSP_NS寄存器到R2（通过VTOR基址+0x00获取MSP_NS）
	  "LDR R2, =0xE000ED00\n"  // VTOR基址（向量表起始地址）
	  "LDR R2, [R2]\n"         // 读取MSP_NS的值

	  // 跳转到C函数进行后续处理
	  "B save_log\n"
  );
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  __asm volatile (
	  "MOV R0, LR\n"

	  "MRS R1, MSP\n"

	  // 读取MSP_NS寄存器到R2（通过VTOR基址+0x00获取MSP_NS）
	  "LDR R2, =0xE000ED00\n"  // VTOR基址（向量表起始地址）
	  "LDR R2, [R2]\n"         // 读取MSP_NS的值

	  // 跳转到C函数进行后续处理
	  "B save_log\n"
  );
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Secure Fault exception.
  * @param  None
  * @retval None
  */
void SecureFault_Handler(void)
{
  __asm volatile (
	  "MOV R0, LR\n"

	  "MRS R1, MSP\n"

	  // 读取MSP_NS寄存器到R2（通过VTOR基址+0x00获取MSP_NS）
	  "LDR R2, =0xE000ED00\n"  // VTOR基址（向量表起始地址）
	  "LDR R2, [R2]\n"         // 读取MSP_NS的值

	  // 跳转到C函数进行后续处理
	  "B save_log\n"
  );

  if(pSecureFault_Callback != (funcptr_NS)NULL)
  {
    if ( pSecureFault_pData != NULL )
    {
      Fault_Info_t *info = (Fault_Info_t *)pSecureFault_pData;
      info->cfsr = SCB->CFSR;
      info->cfsr_ns = SCB_NS->CFSR;
      info->mmfar = SCB->MMFAR;
      info->bfar = SCB->BFAR;
      info->sfsr = SAU->SFSR;
      info->sfar = SAU->SFSR;
      info->tamp_sr = TAMP->SR;
    }
   /* return function pointer with cleared LSB */
   callback_NS = (funcptr_NS)cmse_nsfptr_create(pSecureFault_Callback);

   callback_NS();
  }
  else
  {
    GPIO_InitTypeDef  gpio_init_structure;
    __HAL_RCC_GPIOG_CLK_ENABLE();

    /* configure the GPIOG13 pin */
    gpio_init_structure.Pin = GPIO_PIN_10;
    gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init_structure.Pull = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init (GPIOG, &gpio_init_structure);
  }

  while (1)
  {
    for(int i = 0; i<20000000; i++);
    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_10);
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
  //while (1)
  {
  }
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32N6xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32n6xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles IAC interrupt request.
  * @retval None
  */
void IAC_IRQHandler(void)
{
#if (NS_DATA_IMAGE_NUMBER == 1)
  uint32_t *pfootprint = (uint32_t*)(LOG_RAM_BASE_ADD+0x200);
  *pfootprint = (uint32_t)pfootprint;
#endif

  if(pSecureError_Callback != (funcptr_NS)NULL)
  {
    if ( pSecureError_pData != NULL )
    {
      IAC_Info_t *iac = (IAC_Info_t *)pSecureError_pData;
      iac->iac_iisr[0] = *(volatile uint32_t *)(IAC_BASE + 0x36C);
      iac->iac_iisr[1] = *(volatile uint32_t *)(IAC_BASE + 0x36C+4);
      iac->iac_iisr[2] = *(volatile uint32_t *)(IAC_BASE + 0x36C+8);
      iac->iac_iisr[3] = *(volatile uint32_t *)(IAC_BASE + 0x36C+12);
      iac->iac_iisr[4] = *(volatile uint32_t *)(IAC_BASE + 0x36C+16);
      iac->iac_iisr[5] = *(volatile uint32_t *)(IAC_BASE + 0x36C+20);
    }
   /* return function pointer with cleared LSB */    
   callback_NS = (funcptr_NS)cmse_nsfptr_create(pSecureError_Callback);

   callback_NS();
  }
  else
  {
    while(1);  /* Something went wrong */
  }
}

