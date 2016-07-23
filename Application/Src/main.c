/**
******************************************************************************
* @file    Templates/Src/main.c 
* @author  MCD Application Team
* @version V1.0.3
* @date    06-May-2016
* @brief   Main program body
******************************************************************************
*/
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
static OS_STK App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE-1];
static OS_STK App_MainTaskStk[APP_CFG_MAIN_TASK_STK_SIZE-1];
static OS_STK App_MonitorTaskStk[APP_CFG_MONITOR_TASK_STK_SIZE-1];
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void AppTaskStart (void *p_arg);
static void AppMainTask (void *p_arg);
static void AppMonitorTask (void *p_arg);

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Main program
* @param  None
* @retval None
*/
int main(void)
{
#if (OS_TASK_NAME_EN > 0u)
	uint8_t os_err; 
#endif

	HAL_Init();   
	
	OSInit();
	
	/*创建起始任务*/
	OSTaskCreate(AppTaskStart,
					 0,
					 &App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE-1],
					 APP_CFG_TASK_START_PRIO);
	
#if (OS_TASK_NAME_EN > 0u)
	OSTaskNameSet(APP_CFG_TASK_START_PRIO,
					  "App_Task_Start",
					  &os_err); 
#endif
  
	OSStart();
  
  while (1)
  {
	 ;
  }
}
/**
 * [AppTaskStart  起始任务函数]
 * @param p_arg [输入参数]
 * @note	该函数主要初始化外设，创建主要任务，完成后该任务挂起
 *       死机后可重启该任务。
 */
static void AppTaskStart (void *p_arg)
{
	p_arg = p_arg;
  
#if (OS_TASK_NAME_EN >0u)
	uint8_t os_err; 
#endif
	/* 配置系统主时钟为180 MHz */
	SystemClock_Config();

	/*初始化SDRAM*/
	SDRAM_Init();

	/*初始化GUI图形库*/	
//	GUI_Init();
	
	/*初始化LED外设*/
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);

	/*初始用户按键*/
	BSP_Button_Init(User_Button,Button_Mode_GPIO);
	
	/*建立统计任务*/
	OSStatInit ();
	
	/*创建主任务*/
	OSTaskCreate(AppMainTask,
					 (void *)0,
					 &App_MainTaskStk[APP_CFG_MAIN_TASK_STK_SIZE-1],
					 APP_CFG_MAIN_TASK_PRIO);
	
#if (OS_TASK_NAME_EN >0u)
	OSTaskNameSet(APP_CFG_MAIN_TASK_PRIO,
					  "APP_Main_Task",
					  &os_err);
#endif
	/*创建一个监视任务*/
		OSTaskCreate(AppMonitorTask,
					 (void *)0,
					 &App_MonitorTaskStk[APP_CFG_MAIN_TASK_STK_SIZE-1],
					 APP_CFG_MONITOR_TASK_PRIO);
	
#if (OS_TASK_NAME_EN >0u)
	OSTaskNameSet(APP_CFG_MONITOR_TASK_PRIO,
					  "APP_Monitor_Task",
					  &os_err);
#endif
	/*挂起启动任务*/
	OSTaskSuspend(OS_PRIO_SELF);
	
	for(;;)
	{
	  ;
	}
}
/**
 * [AppMainTask  主任务函数]
 * @param p_arg [参数]
 */
static void AppMainTask (void *p_arg)
{
	p_arg = p_arg;
	for(;;)
	{
		BSP_LED_Toggle(LED2);
		OSTimeDly(100);
		if(BSP_Button_GetState(User_Button) == (uint32_t)1)
		{
			OSTimeDly(10);
			if(BSP_Button_GetState(User_Button) == (uint32_t)1)
			{
			BSP_LED_Toggle(LED3);
			while(BSP_Button_GetState(User_Button) == 1);
			}
		}
	}	
}
static void AppMonitorTask (void *p_arg)
{
	p_arg = p_arg;
	for(;;)
	{
		BSP_LED_Toggle(LED1);
		OSTimeDly(1000);
	}
}
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
