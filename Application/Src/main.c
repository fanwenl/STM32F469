/**
******************************************************************************
* @file    Templates/Src/main.c 
* @author  MCD Application Team
* @version V1.0.3
* @date    06-May-2016
* @brief   Main program body
******************************************************************************

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F4xx_HAL_Examples
* @{
*/

/** @addtogroup Templates
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//static OS_STK App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE-1];
//static OS_STK App_MainTaskStk[APP_CFG_MAIN_TASK_STK_SIZE-1];
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
//static void AppTaskStart (void *p_arg);
//static void AppMainTask (void *p_arg);

#define  SDRAM_SIZE  ((uint32_t)0x8000)
uint32_t a[SDRAM_SIZE];
uint32_t b[SDRAM_SIZE];
extern uint32_t uwTick;
uint8_t  c[] = {'A'};

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Main program
* @param  None
* @retval None
*/
int main(void)
{
  uint32_t i, j;
#if (OS_TASK_NAME_EN > 0u)
  uint8_t os_err; 
#endif
  /* STM32F4xx HAL library initialization*/
  HAL_Init();
  
  SystemClock_Config();
  
  SDRAM_Init();
  /*LCD_ORIENTATION_LANDSCAPE  LCD_ORIENTATION_PORTRAIT*/
  LCD_Init(LCD_ORIENTATION_PORTRAIT);
  
 LCD_LayerDefaultInit(0,SDRAM_BANK1_ADDR);
 
//  LCD_SelectLayer(0);
  
	LCD_DisplayChar(0, 0, 'A');
	LCD_DisplayChar(480, 800, 'B');
 

  
  for(i = 0; i <= SDRAM_SIZE;i++)
  {
	 a[i] =(uint32_t) SDRAM_SIZE + i;
  }	
  if(SDRAM_Init() != SDRAM_ERROR)
  {
	 printf("%s\n","sdram init ok");
	 uwTick = 0;
	 printf("%ld\n",uwTick);
	 SDRAM_WriteData(SDRAM_DEVICE_ADDR, a, (uint32_t)SDRAM_SIZE);
	 SDRAM_WriteData(SDRAM_DEVICE_ADDR, a, (uint32_t)SDRAM_SIZE);
	 SDRAM_WriteData(SDRAM_DEVICE_ADDR, a, (uint32_t)SDRAM_SIZE);
	 SDRAM_WriteData(SDRAM_DEVICE_ADDR, a, (uint32_t)SDRAM_SIZE);
	 printf("%ld\n",uwTick);
	 
	 SDRAM_ReadDataDMA(SDRAM_DEVICE_ADDR, b, (uint32_t)10);
	 for (j = 0; j < 10; j++)
	 {  
		printf("%d\n",b[j]);
	 }
  }
  else
  {
	 printf("%s\n","sdram error");
  }
  
  //	OSInit();
  //	
  //	OSTaskCreate(AppTaskStart,
  //					 0,
  //					 &App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE-1],
  //					 APP_CFG_TASK_START_PRIO);
  //#if (OS_TASK_NAME_EN > 0u)
  //	OSTaskNameSet(APP_CFG_TASK_START_PRIO,
  //					  "App_Task_Start",
  //					  &os_err); 
  //#endif
  //
  //	OSStart();
  
  while (1)
  {
	 ;
  }
  
}
//static void AppTaskStart (void *p_arg)
//{
//  p_arg = p_arg;
//  
//#if (OS_TASK_NAME_EN >0u)
//	uint8_t os_err; 
//#endif
// /* Configure the system clock to 180 MHz */
//  	SystemClock_Config();
//	LED_Init(LED1);
//	LED_Init(LED2);
//	LED_Init(LED3);
//	LED_Init(LED4);
//	Button_Init(User_Button,Button_Mode_GPIO);
//	LED_On(LED1);
//	OSStatInit ();
//	OSTaskCreate(AppMainTask,
//					 (void *)0,
//					 &App_MainTaskStk[APP_CFG_MAIN_TASK_STK_SIZE-1],
//					 APP_CFG_MAIN_TASK_PRIO);
//#if (OS_TASK_NAME_EN >0u)
//	OSTaskNameSet(APP_CFG_MAIN_TASK_PRIO,
//					  "APP_Main_Task",
//					  &os_err);
//	OSTaskSuspend(OS_PRIO_SELF);
//	for(;;)
//	{
//	  ;
//	}
//#endif
//}
//static void AppMainTask (void *p_arg)
//{
//  	p_arg = p_arg;
//	for(;;)
//	{
//		LED_Toggle(LED2);
//		HAL_Delay(10);
//		if(Button_GetState(User_Button) == (uint32_t)1)
//		{
//			HAL_Delay(10);
//			if(Button_GetState(User_Button) == (uint32_t)1)
//			{
//			LED_Toggle(LED3);
//			while(Button_GetState(User_Button) == 1);
//			}
//	 	}
//	}	
//}

/*
***************************************************************************************
* @brief  System Clock Configuration
*         The system Clock is configured as follow : 
*            System Clock source            = PLL (HSE)
*            SYSCLK(Hz)                     = 180000000
*            HCLK(Hz)                       = 180000000
*            AHB Prescaler                  = 1
*            APB1 Prescaler                 = 4
*            APB2 Prescaler                 = 2
*            HSE Frequency(Hz)              = 8000000
*            PLL_M                          = 8
*            PLL_N                          = 360
*            PLL_P                          = 2
*            PLL_Q                          = 8
*            PLL_R                          = 2
*            VDD(V)                         = 3.3
*            Main regulator output voltage  = Scale1 mode
*            Flash Latency(WS)              = 5
* @param  None
* @retval None
*************************************************************************************************
*/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;         
  RCC_OscInitTypeDef RCC_OscInitStruct;         
  
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
  clocked below the maximum system frequency, to update the voltage scaling value 
  regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  
  RCC_OscInitStruct.PLL.PLLM = 8;                    /*PLLM  1MHZ*/
  RCC_OscInitStruct.PLL.PLLN = 360;                  /*PLLN 360MHz*/
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;                    /*PLLQ  45MHZ*/
  RCC_OscInitStruct.PLL.PLLR = 2;                    /*PLLR 180MHZ*/
  
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
	 Error_Handler();
  }
  /* Enable the OverDrive to reach the 180 Mhz Frequency */  
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
	 Error_Handler();
  } 
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;            /*HCLK 180MHz*/
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;             /*APB1 45MHz*/
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;             /*APB2 90MHz*/
  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
	 Error_Handler();
  }
}
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  /*Configure the SysTick IRQ priority */
  HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority ,0U);
  //  if(OSRunning > 0u)
  //  {
  /*Configure the SysTick to have interrupt in 1ms time basis*/
  HAL_SYSTICK_Config(SystemCoreClock/10000U);
  //  }
  /* Return function status */
  return HAL_OK;
}

/**
* @brief  This function is executed in case of error occurrence.
* @param  None
* @retval None
*/
static void Error_Handler(void)
{
  /* User may add here some code to deal with this error */
  while(1)
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
