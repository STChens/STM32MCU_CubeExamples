/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    HASH/HASH_SHA256/Src/main.c
  * @author  MCD Application Team
  * @brief   This example provides a description of how to use the HASH peripheral to 
  *          hash data using SHA_256 Algorithm with STM32H5xx.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h5xx_nucleo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AppVer 'A'
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

HASH_HandleTypeDef hhash;

/* USER CODE BEGIN PV */
__ALIGN_BEGIN const uint8_t aInput[] __ALIGN_END = "STM32H5 is a high-performance microcontrollers family based on Arm Cortex-M33 32-bit RISC core. It offers native and installable security services.";

__ALIGN_BEGIN uint8_t aSHA256Digest[32] __ALIGN_END;
__ALIGN_BEGIN uint8_t aExpectSHA256Digest[32] __ALIGN_END = {0x02, 0xfc, 0xa4, 0x29, 0x66, 0x37, 0xc1, 0xb0, 0xc2,
                                                                    0x8a, 0xbb, 0xe4, 0xc4, 0x15, 0x40, 0xfb, 0x8c, 0x0a,
                                                                    0xb2, 0x48, 0xf1, 0xf9, 0x99, 0xa0, 0x16, 0x81, 0xd1,
                                                                    0xe5, 0x1c, 0x70, 0xfe, 0x7c
							            };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_HASH_Init(void);
static void MX_ICACHE_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//#define TEST_WITH_INTERRUPT 1

//#define BUFFLEN 4
//#define BUFFLEN 16
//#define BUFFLEN 32
//#define BUFFLEN 64
//#define BUFFLEN 96
//#define BUFFLEN 128
#define BUFFLEN 256

#if ((USE_BSP_COM_FEATURE > 0)&&(USE_COM_LOG > 0))
#define LOG_ENABLED
#endif

uint16_t currLen;
uint16_t inputBuffLen;
uint16_t nofSubBuffers;

#if defined LOG_ENABLED
static void print_buf(char *str, const uint8_t *buf, int size)
{
    int i;
    printf("\r\n=========================================\r\n");
    if (str != NULL)
    {
            printf("%s\r\n",str);
    }
    for(i = 0; i< size; i++)
    {
            printf("%02x ", buf[i]);
            if((i+1)%16 == 0)
            {
                    printf("\r\n");
            }
    }
    printf("\r\n=========================================\r\n");
}

static void print_hashtest_menu(void)
{
	printf("\r\n===============================================\r\n");
	printf("Please choose the buffer length for HASH test \r\n\r\n");
	printf("Buffer length 4:  -------------- 1\r\n");
	printf("Buffer length 8:  -------------- 2\r\n");
	printf("Buffer length 16:  ------------- 3\r\n");
	printf("Buffer length 32:  ------------- 4\r\n");
	printf("Buffer length 64:  ------------- 5\r\n");
	printf("Buffer length 128:  ------------ 6\r\n");
	printf("Buffer length 256:  ------------ 7\r\n");
#if defined USE_FREE_RTOS    
        printf("Test in multi thread:  --------- 8\r\n");
#endif
        printf("Return:  ----------------------- x\r\n");
	printf("================================================\r\n");
}

static void print_main_menu(void)
{
	printf("\r\nJump to loader for App FW update:  -------------- 1\r\n");
	printf("\r\nRun HASH test:  --------------------------------- 2\r\n");
	printf("\r\n=======================================================\r\n");	
}
#endif

#define HASH_NUMBER_OF_CSR_REGISTERS_TEST              103U
uint8_t HashBuffer[(HASH_NUMBER_OF_CSR_REGISTERS_TEST + 3) * 4];

__ALIGN_BEGIN static uint8_t Input[1024*200] __ALIGN_END ={0};
__ALIGN_BEGIN static uint8_t aDigest1[48] __ALIGN_END;
__ALIGN_BEGIN static uint8_t aDigest2[48] __ALIGN_END;
__ALIGN_BEGIN static uint8_t aExpectedDigest[48] __ALIGN_END =
{
        0x16,0x3d,0x65,0x47,0x91,0xc8,0x8a,0x0d,0x46,0x31,0x22,0xa4,0xbc,0xa2,0xc0,0xe6,
        0xdf,0x2c,0xc5,0xf1,0xfc,0xdc,0xb8,0x8b,0x1e,0xf0,0xf8,0x01,0x6c,0xfd,0xbd,0xd9,
        0x4c,0x6b,0xad,0x1d,0x38,0x07,0xd9,0x24,0xd9,0xf9,0x14,0x79,0x85,0xdc,0xc3,0x7b
};

static void SHA256_SuspendResume_Test(void)
{
  hhash.Instance = HASH;
  hhash.Init.DataType = HASH_BYTE_SWAP;
  hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA384;
  
  if (HAL_HASH_Init(&hhash) != HAL_OK)
  {
    Error_Handler();
  }
  
  int i;
  for(i=0; i<sizeof(Input); i ++)
  {
	  Input[i] =i&0xff;
  }
  printf("Test1 Start.Bytes[%d]\r\n",sizeof(Input));
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
  if (HAL_HASH_Start(&hhash, (uint8_t *)Input, sizeof(Input), aDigest1, 0xFF) != HAL_OK)
  {
    Error_Handler();
  }  
  
  printf("HASH digest completed.\r\n");

  print_buf("HASH_Start result", aDigest1,sizeof(aDigest1));

   /* Check the output buffer containing the computing digest with the expected buffer */
  if(memcmp(aDigest1, aExpectedDigest,48) != 0)
  {
      printf("the computed digest1 result is not match the expected value.\r\n");
      Error_Handler();
  }
  printf("Test1 finished. It runs well.\r\n");
  
  // 132 = 4 * 32 + 4
  int total_size = sizeof(Input);
  int accum_size = ((total_size/136))*136;
  int accum_last_size = total_size - accum_size;
  
  printf("Test2 Accumulate.Bytes[%d]\r\n",accum_size);
  if (HAL_HASH_Accumulate(&hhash, (uint8_t *)Input, accum_size, HAL_MAX_DELAY) != HAL_OK)
  {
    Error_Handler();
  }
#if (USE_HAL_HASH_SUSPEND_RESUME == 1U)    
  HAL_HASH_Suspend(&hhash,HashBuffer);
  HAL_HASH_Resume(&hhash,HashBuffer);
#endif
  printf("Test2 AccumulateLast.Bytes[%d]\r\n",accum_last_size);
  if (HAL_HASH_AccumulateLast(&hhash, (uint8_t *)Input + accum_size, accum_last_size, aDigest2, HAL_MAX_DELAY) != HAL_OK)
  {
        Error_Handler();
  }
  printf("HASH digest2 completed.\r\n");
  print_buf("Accumulate test result",aDigest2,sizeof(aDigest2));
  if(memcmp(aDigest2, aExpectedDigest,48) != 0)
  {
      printf("the computed digest2 result is not match the expected value.\r\n");
      Error_Handler();
  }
  printf("Test2 finished. It runs well.\r\n");
  
  HAL_HASH_DeInit(&hhash);
}

static void SHA256_Accumulate_Test(HASH_HandleTypeDef *phhash, int bufLen)
{
  int hashChunkIndex = 0;
#if (USE_HAL_HASH_SUSPEND_RESUME == 1U)   
  uint8_t suspended = 0;
  uint8_t ctx_buf[106*4];
#endif /* USE_HAL_HASH_SUSPEND_RESUME */ 
  
#if defined LOG_ENABLED
  printf("HASH Test start...\r\n");
  print_buf("Data to be hashed:", aInput, strlen((char const*)aInput));
  printf("Buffer length for the test: %d\r\n", bufLen);
#endif

  currLen = 0;
  inputBuffLen = strlen((char const*)aInput);
  nofSubBuffers = (inputBuffLen + (bufLen-1))/bufLen;
  for (hashChunkIndex = 0; hashChunkIndex < (nofSubBuffers-1); hashChunkIndex++)
  {
#if (USE_HAL_HASH_SUSPEND_RESUME == 1U)  
    if( suspended == 1)
    {
      HAL_HASH_Resume(phhash, &ctx_buf[0]);
      suspended = 0;
    }
#endif /* USE_HAL_HASH_SUSPEND_RESUME */   
    if (HAL_HASH_Accumulate(phhash, (uint8_t*)&aInput[hashChunkIndex*bufLen], bufLen ,HAL_MAX_DELAY) != HAL_OK)
    {
            Error_Handler();
    }
    currLen += bufLen;
#if (USE_HAL_HASH_SUSPEND_RESUME == 1U)        
    if( suspended == 0)
    {
      HAL_HASH_Suspend(phhash, &ctx_buf[0]);
      suspended = 1;
    }
#endif /* USE_HAL_HASH_SUSPEND_RESUME */   
  }

#if (USE_HAL_HASH_SUSPEND_RESUME == 1U)      
  if( suspended == 1)
  {
    HAL_HASH_Resume(phhash, &ctx_buf[0]);
    suspended = 0;
  }
#endif /* USE_HAL_HASH_SUSPEND_RESUME */  
  if (HAL_HASH_AccumulateLast(phhash, (uint8_t*)&aInput[(nofSubBuffers-1)*bufLen], (inputBuffLen-currLen), aSHA256Digest, HAL_MAX_DELAY) != HAL_OK)
  {
      Error_Handler();
  }
  
 /* Compare computed digest with expected one */
  if(memcmp(aSHA256Digest, aExpectSHA256Digest, sizeof(aExpectSHA256Digest)/sizeof(aExpectSHA256Digest[0])) != 0)
  {
    Error_Handler();
  }
#if defined LOG_ENABLED
  else
  {
      print_buf("Computed HASH:", aSHA256Digest, sizeof(aSHA256Digest));
  }
  printf("HASH Test done.\r\n");
#endif
}

static void Run_HASH_TEST(void)
{  
  int exit = 0;
  MX_HASH_Init();
  
#if defined LOG_ENABLED
  while(exit == 0)
  {
	  uint8_t select = 0;

	  extern UART_HandleTypeDef hcom_uart[COM_NBR];

	  print_hashtest_menu();

	  while(1)
	  {
		  if ( HAL_UART_Receive(&hcom_uart[COM1], &select, sizeof(select), 1000) == HAL_OK)
			  break;
	  }
	  printf("Your choice is %c\r\n", select);
	  switch(select)
	  {
	  case '1':
		  SHA256_Accumulate_Test(&hhash, 4*17);
	  	  break;
	  case '2':
		  SHA256_Accumulate_Test(&hhash,8*17);
	  	  break;
	  case '3':
		  SHA256_Accumulate_Test(&hhash,16);
	  	  break;
	  case '4':
		  SHA256_Accumulate_Test(&hhash,32);
	  	  break;
	  case '5':
		  SHA256_Accumulate_Test(&hhash,64);
	  	  break;
	  case '6':
		  SHA256_Accumulate_Test(&hhash,128);
	  	  break;
	  case '7':
		  SHA256_Accumulate_Test(&hhash,256);
	  	  break;
#if defined USE_FREE_RTOS                  
          case '8':
		  FreeRTOS_Test_Run();
	  	  break;
#endif
          case 'x':
                  exit = 1;
                  break;
	  default:
		  printf("Invalid input!\r\n");
		  break;
	  }
  }
#else
  SHA256_Accumulate_Test(BUFFLEN);
#endif /* USE_COM_LOG */   
}

struct boot_arm_vector_table {
    uint32_t msp;
    uint32_t reset;
};

static void Jump2Loader(void)
{
  static struct boot_arm_vector_table *vt;
  
  vt = (struct boot_arm_vector_table *)BOOTLOADER_BASE;
  /*  change stack limit  */
  __set_MSPLIM(0);  
  __set_MSP(vt->msp);
  void (*fp)(void) = (void(*)(void))vt->reset;
  fp();
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* STM32H5xx HAL library initialization:
       - Systick timer is configured by default as source of time base, but user
             can eventually implement his proper time base source (a general purpose
             timer for example or other time source), keeping in mind that Time base
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
             handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  HAL_MPU_Disable();

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  
  MX_ICACHE_Init();
  /* USER CODE BEGIN 2 */
  /* Configure LED1, LED3 */
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED3);

#if defined LOG_ENABLED
  COM_InitTypeDef COM_Init;
  COM_Init.BaudRate = 115200;
  COM_Init.HwFlowCtl = COM_HWCONTROL_NONE;
  COM_Init.Parity = COM_PARITY_NONE;
  COM_Init.StopBits = COM_STOPBITS_1;
  COM_Init.WordLength = COM_WORDLENGTH_8B;

  BSP_COM_Init(COM1, &COM_Init);
  printf("\r\n=======================================================\r\n");
  printf("= Test Application version: (%c) \r\n", AppVer);
  printf("= Build time: %s %s\r\n", __DATE__, __TIME__);
  printf("=======================================================\r\n");
  
  MX_HASH_Init();

  while(1)
  {
	  uint8_t select = 0;

	  extern UART_HandleTypeDef hcom_uart[COM_NBR];

	  print_main_menu();

	  while(1)
	  {
		  if ( HAL_UART_Receive(&hcom_uart[COM1], &select, sizeof(select), 1000) == HAL_OK)
			  break;
	  }
	  printf("Your choice is %c\r\n", select);
	  switch(select)
	  {
	  case '1':
		  Jump2Loader();
	  	  break;
	  case '2':
		  Run_HASH_TEST();
	  	  break;
	  default:
		  printf("Invalid input!\r\n");
		  break;
	  }
  }

#endif /* USE_COM_LOG */ 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DIGITAL;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 250;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the programming delay
  */
  __HAL_FLASH_SET_PROGRAM_DELAY(FLASH_PROGRAMMING_DELAY_2);
}

/**
  * @brief HASH Initialization Function
  * @param None
  * @retval None
  */
static void MX_HASH_Init(void)
{

  /* USER CODE BEGIN HASH_Init 0 */

  /* USER CODE END HASH_Init 0 */

  /* USER CODE BEGIN HASH_Init 1 */

  /* USER CODE END HASH_Init 1 */
  hhash.Instance = HASH;
  hhash.Init.DataType = HASH_BYTE_SWAP;
  hhash.Init.Algorithm = HASH_ALGOSELECTION_SHA256;
  if (HAL_HASH_Init(&hhash) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN HASH_Init 2 */

  /* USER CODE END HASH_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
  */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
#if defined LOG_ENABLED
  printf("%s:%d\r\n", __FUNCTION__, __LINE__);
#endif  
  /* User can add his own implementation to report the HAL error return state */
  while (1)
  {
    /* Toggle LED3 with a period of 200 ms */ 
    BSP_LED_Toggle(LED3);
    HAL_Delay(200);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
