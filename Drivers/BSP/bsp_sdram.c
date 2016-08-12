/*
*************************************************************************************************
*文件：bsp_sdarm.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-06-04
*描述：STM32F469NI-Discovery开发套件SDARN驱动程序。
**************************************************************************************************
*/
#include "bsp_sdram.h"

/*
**************************************************************************************************
*										SDRAM公用结构体定义
**************************************************************************************************
 */
static SDRAM_HandleTypeDef SDRAM_Handle;
static FMC_SDRAM_TimingTypeDef  TimStructure;
/*
**************************************************************************************************
*										SDRAM驱动私有函数申明
**************************************************************************************************
 */
static void BSP_SDRAM_InitSeq(uint32_t RefreshCount);
static void BSP_SDRAM_MspInit(SDRAM_HandleTypeDef *HSDRAM);
/*
**************************************************************************************************
*描述：SDARM初始化函数
*参数：无
*返回：返回SDRAM初始化状态
*note：该函数初始化FMC时序以及SDRAM
* ************************************************************************************************
 */
uint8_t BSP_SDRAM_Init(void)
{
	static uint8_t sdramstatus = SDRAM_ERROR;

	/*配置SDRAM外设寄存器的地址*/
	SDRAM_Handle.Instance = FMC_SDRAM_DEVICE;

	/*配置FMC的SDRAM时序*/
	TimStructure.LoadToActiveDelay    = 2;		/*从加载模式寄存器到active的延时*/
	TimStructure.ExitSelfRefreshDelay = 7;		/*退出自己刷新的延时*/
	TimStructure.SelfRefreshTime      = 7;		/*自动刷新最小周期*/
	TimStructure.RowCycleDelay        = 7;		/*两个连续刷新命令之间的延时*/
	TimStructure.WriteRecoveryTime    = 2;		/*写恢复的时间*/
	TimStructure.RPDelay              = 2;		/*预充电命令的延时*/
	TimStructure.RCDDelay             = 2;		/*激活命令和读写命令之间的延时*/

	/*配置SDRAM*/
	SDRAM_Handle.Init.SDBank             = FMC_SDRAM_BANK1;							/*SDRAM挂载FMC的Bank1*/
	SDRAM_Handle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;			/*列地址为8bit*/
	SDRAM_Handle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;				/*行地址为11bit*/
	SDRAM_Handle.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;			/*SDRAM数据宽度为32bit*/
	SDRAM_Handle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;			/*SDRAM芯片包还四个Bank*/
	SDRAM_Handle.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;				/*SDRAM芯片的CAS延时为3个周期*/
	SDRAM_Handle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;	/*禁止写保护功能*/
	/*SDRAM的周期为HCLK的2倍，HCLK为180M，所以SDRAM的频率为90M*/
	SDRAM_Handle.Init.SDClockPeriod      =	FMC_SDRAM_CLOCK_PERIOD_2;				
	SDRAM_Handle.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;				/*使能突发模式*/
	SDRAM_Handle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;				/*这个延时为0*/
	
	/*初始化SDRAM的MSP外设*/
	BSP_SDRAM_MspInit(&SDRAM_Handle);
	
	if (HAL_SDRAM_Init(&SDRAM_Handle,&TimStructure) != HAL_OK)
	{
		sdramstatus = SDRAM_ERROR;
	}
	else
	{
		sdramstatus = SDRAM_OK;
	}
	/*初始化SDRAM的通道*/
	BSP_SDRAM_InitSeq(REFRESH_COUNT);

	return sdramstatus;
}
/*
**************************************************************************************************
*描述：SDARM恢复默认
*参数：无
*返回：返回程序的状态
*note：该函数将SDRAM恢复到默认
* ************************************************************************************************
 */
uint8_t BSP_SDRAM_DeInit(void)
{
	static uint8_t sdramstatus = SDRAM_ERROR;

	SDRAM_Handle.Instance = FMC_SDRAM_DEVICE;
	
	if(HAL_SDRAM_DeInit(&SDRAM_Handle) == HAL_OK)
  {
		sdramstatus = SDRAM_OK;

		BSP_SDRAM_MspDeInit(&SDRAM_Handle);
  }
  return sdramstatus;
}
/*
**************************************************************************************************
*描述：SDARM芯片初始化函数
*参数：RefreshCount SDRAM刷新计数器
*返回：无
*note：无
* ************************************************************************************************
 */
static void BSP_SDRAM_InitSeq(uint32_t RefreshCount)
{
	static FMC_SDRAM_CommandTypeDef CommandStructure;
	static uint32_t temp = 0;

	/*配置时钟信号，并且使能时钟*/
	CommandStructure.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;		/*时钟使能命令*/
	CommandStructure.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;	/*FMC bank1目标*/
	CommandStructure.AutoRefreshNumber      = 1;										/*连续刷新的次数在刷新模式中设置*/
	CommandStructure.ModeRegisterDefinition = 0;										/*模式寄存器的值*/

	HAL_SDRAM_SendCommand(&SDRAM_Handle, &CommandStructure, SDRAM_TIME_OUT);

	/*延时100us*/
	HAL_Delay(1);

	/*发送预充电命令*/
	CommandStructure.CommandMode            = FMC_SDRAM_CMD_PALL;				/*预充电命令*/
	CommandStructure.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;	/*FMC bank1目标*/
	CommandStructure.AutoRefreshNumber      = 1;										/*连续刷新的次数在刷新模式中设置*/
	CommandStructure.ModeRegisterDefinition = 0;										/*模式寄存器的值*/

	HAL_SDRAM_SendCommand(&SDRAM_Handle, &CommandStructure, SDRAM_TIME_OUT);

	/*发送自动刷新命令*/
	CommandStructure.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;/*自动刷新命令*/
	CommandStructure.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;	/*FMC bank1目标*/
	CommandStructure.AutoRefreshNumber      = 8;										/*连续刷新的次数在刷新模式中设置*/
	CommandStructure.ModeRegisterDefinition = 0;										/*模式寄存器的值*/

	HAL_SDRAM_SendCommand(&SDRAM_Handle, &CommandStructure, SDRAM_TIME_OUT);

	/*配置模式寄存器，单次突发模式，连续突发读取，设置CAS为3，标准操作模式，单次写入模式*/
	temp = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1         |\
	  					  SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL  |\
		 				  SDRAM_MODEREG_CAS_LATENCY_3          |\
						  SDRAM_MODEREG_OPERATING_MODE_STANDARD|\
			  			  SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED;					

	CommandStructure.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;		/*模式寄存器命令*/
	CommandStructure.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;	/*FMC bank1目标*/
	CommandStructure.AutoRefreshNumber      = 1;										/*连续刷新的次数在刷新模式中设置*/
	CommandStructure.ModeRegisterDefinition = temp;									/*模式寄存器的值*/

	HAL_SDRAM_SendCommand(&SDRAM_Handle, &CommandStructure, SDRAM_TIME_OUT);

	/*设置刷新率寄存器*/
	HAL_SDRAM_ProgramRefreshRate(&SDRAM_Handle, RefreshCount);
}
/*
**************************************************************************************************
*描述：SDARM读数据函数
*参数：pAddress			SDRAM地址
*		 pDstBuffer			读出数据的的目标地址
*		 BufferSize			读出数据的大小
*返回：返回读数据的状态
*note：数据的大小为32bit
* ************************************************************************************************
 */
uint8_t BSP_SDRAM_ReadData( uint32_t pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
	if (HAL_SDRAM_Read_32b(&SDRAM_Handle, (uint32_t *)pAddress, pDstBuffer, BufferSize) != HAL_OK)
	{
		return SDRAM_ERROR;
	}
	else
	{
		return SDRAM_OK;
	}
}
/*
**************************************************************************************************
*描述：SDARM使用DMA读数据函数
*参数：pAddress			SDRAM地址
*		 pDstBuffer			读出数据的的目标地址
*		 BufferSize			读出数据的大小
*返回：返回读数据的状态
*note：数据的大小为32bit，
*		 需要使能DMA2外设以及DAM的中断
* ************************************************************************************************
 */
uint8_t BSP_SDRAM_ReadDataDMA( uint32_t pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
	if (HAL_SDRAM_Read_DMA(&SDRAM_Handle, (uint32_t *)pAddress, pDstBuffer, BufferSize) != HAL_OK)
	{
		return SDRAM_ERROR;
	}
	else
	{
		return SDRAM_OK;
	}
}
/*
**************************************************************************************************
*描述：SDARM写数据函数
*参数：pAddress			SDRAM的目标地址
*		 pDstBuffer			写入数据的缓存地址
*		 BufferSize			写入数据的大小
*返回：返回写数据的状态
*note：数据的大小为32bit，
* ************************************************************************************************
 */
uint8_t BSP_SDRAM_WriteData(uint32_t pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
	if (HAL_SDRAM_Write_32b(&SDRAM_Handle, (uint32_t *)pAddress, pDstBuffer, BufferSize) != HAL_OK)
	{
		return SDRAM_ERROR;
	}
	else
	{
		return SDRAM_OK;
	}
}

/*
**************************************************************************************************
*描述：SDARM使用DMA写数据函数
*参数：pAddress			SDRAM的目标地址
*		 pDstBuffer			写入数据的缓存地址
*		 BufferSize			写出数据的大小
*返回：返回写数据的状态
*note：数据的大小为32bit，
*		 需要使能DMA2外设以及DAM的中断
* ************************************************************************************************
 */
uint8_t BSP_SDRAM_WriteDataDMA(uint32_t pAddress, uint32_t *pDstBuffer, uint32_t BufferSize)
{
	if (HAL_SDRAM_Write_DMA(&SDRAM_Handle, (uint32_t *)pAddress, pDstBuffer, BufferSize) != HAL_OK)
	{
		return SDRAM_ERROR;
	}
	else
	{
		return SDRAM_OK;
	}
}
/*
**************************************************************************************************
*描述：发送SDRAM控制命令
*参数：SdramCmd FMC SDRAM 命令结构体
*返回：返回发送命令的状态
*note：无
* ************************************************************************************************
 */
uint8_t BSP_SDRAM_SendCmd(FMC_SDRAM_CommandTypeDef *SdramCmd)
{
	if (HAL_SDRAM_SendCommand(&SDRAM_Handle, SdramCmd, SDRAM_TIME_OUT) != HAL_OK)
	{
		return SDRAM_ERROR;
	}
	else
	{
		return SDRAM_OK;
	}
}
/*
**************************************************************************************************
*描述：SDRAM 申请DMA中断函数
*参数：无
*返回：无
*note：无
* ************************************************************************************************
 */
void SDRAM_DMA_IRQHandler(void)
{
	  HAL_DMA_IRQHandler(SDRAM_Handle.hdma);
}
/*
**************************************************************************************************
*描述：SDARM外设资源初始化
*参数：无
*返回：无
*note：该函数初始化SDARM的GPIO和外设时钟
* ************************************************************************************************
 */
static void BSP_SDRAM_MspInit(SDRAM_HandleTypeDef *HSDRAM)
{
  static DMA_HandleTypeDef DMA_Handle;
  GPIO_InitTypeDef GPIO_InitStructure;

  if(HSDRAM != (SDRAM_HandleTypeDef *)NULL)
	{

		__HAL_RCC_FMC_CLK_ENABLE();							/*使能FMC外设时钟*/

		__HAL_RCC_DMA2_CLK_ENABLE();							/*使能DMA2外设时钟*/

		/*使能GPIO端口时钟*/
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();
		__HAL_RCC_GPIOE_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();
		__HAL_RCC_GPIOI_CLK_ENABLE();
	 	
		/*配置GPIO模式*/
		GPIO_InitStructure.Mode      = GPIO_MODE_AF_PP; 		/*复用推挽模式*/
		GPIO_InitStructure.Pull      = GPIO_PULLUP;				/*上拉*/
		GPIO_InitStructure.Speed     = GPIO_SPEED_FAST;
		GPIO_InitStructure.Alternate = GPIO_AF12_FMC;			/*FMC复用功能*/
	  
		/*GPIOF IO口初始化*/
		GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |\
		 								 GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 |\
		 								 GPIO_PIN_11;
		HAL_GPIO_Init(GPIOF,&GPIO_InitStructure);
		
		/*GPIOG IO口初始化*/
		GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 |GPIO_PIN_5 | GPIO_PIN_8 |\
		 								 GPIO_PIN_15;
		HAL_GPIO_Init(GPIOG,&GPIO_InitStructure);
		
		/*GPIOD IO口初始化*/
		GPIO_InitStructure.Pin = GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 |\
										 GPIO_PIN_9 | GPIO_PIN_10;
		HAL_GPIO_Init(GPIOD,&GPIO_InitStructure);
		
		/*GPIOE IO口初始化*/
		GPIO_InitStructure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |\
		 								 GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 |\
		 								 GPIO_PIN_1 ;
		HAL_GPIO_Init(GPIOE,&GPIO_InitStructure);

		/*GPIOH IO口初始化*/
		GPIO_InitStructure.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |\
		 								 GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 |GPIO_PIN_2 | GPIO_PIN_3;
		HAL_GPIO_Init(GPIOH,&GPIO_InitStructure);

		/*GPIOI IO口初始化*/
		GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 |\
		 								 GPIO_PIN_7 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_4 | GPIO_PIN_5;
		HAL_GPIO_Init(GPIOI,&GPIO_InitStructure);
		
		/*GPIOC IO口初始化*/
		GPIO_InitStructure.Pin = GPIO_PIN_0;
		HAL_GPIO_Init(GPIOC,&GPIO_InitStructure);
		
		/*配置DMA*/
		DMA_Handle.Init.Channel 				= SDRAM_DMA_CHANNEL; 		/*DMA通道选择*/
		DMA_Handle.Init.Direction 				= DMA_MEMORY_TO_MEMORY;		/*DMA方式从memory到memory*/
		DMA_Handle.Init.FIFOMode 				= DMA_FIFOMODE_ENABLE;		/*DAM 使能FIFO模式*/
		DMA_Handle.Init.FIFOThreshold 		= DMA_FIFO_THRESHOLD_FULL;	/*使用全部的FIFO*/
		DMA_Handle.Init.MemBurst 				= DMA_MBURST_SINGLE; 		/*memory单次突发*/
		DMA_Handle.Init.MemDataAlignment		= DMA_MDATAALIGN_WORD;		/*memory数据大小32bit*/
		DMA_Handle.Init.MemInc 					= DMA_MINC_ENABLE; 			/*memory自动增加模式*/
		DMA_Handle.Init.Mode 					= DMA_NORMAL; 					/*DMA正常模式，DMA流控模式*/
		DMA_Handle.Init.PeriphBurst 			= DMA_PBURST_SINGLE;			/*外设单次突发*/
		DMA_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;		/*外设数据大小32bit*/
		DMA_Handle.Init.PeriphInc 				= DMA_PINC_ENABLE; 			/*外设DMA自动增加模式*/
		DMA_Handle.Init.Priority 				= DMA_PRIORITY_HIGH; 		/*DMA优先级高*/
		DMA_Handle.Instance = SDRAM_DMA_STREAM;     /*DMA Stream 寄存器地址*/
		__HAL_LINKDMA(HSDRAM, hdma,DMA_Handle);	  /*链接DAM*/
		
		HAL_DMA_DeInit(&DMA_Handle);					  /*初始化DAM*/
		 
		HAL_DMA_Init(&DMA_Handle);
		
		HAL_NVIC_SetPriority(SDRAM_DMA_IRQn, 5, 0);/*使能DMA中断，并设置优先级*/
	   HAL_NVIC_EnableIRQ(SDRAM_DMA_IRQn);
	}
}
/*
**************************************************************************************************
*描述：SDARM外设资源恢复默认
*参数：无
*返回：无
*note：该函数没有恢复GPIO和外设的时钟，需要用户程序自行恢复。
* ************************************************************************************************
 */
void BSP_SDRAM_MspDeInit(SDRAM_HandleTypeDef *HSDRAM )
{
	static DMA_HandleTypeDef DMA_Handle;

	if(HSDRAM != (SDRAM_HandleTypeDef *)NULL)
	{
		/*禁止SDARM的DMA*/
		HAL_NVIC_DisableIRQ(SDRAM_DMA_IRQn);

		/*恢复默认*/
		DMA_Handle.Instance = SDRAM_DMA_STREAM;
		HAL_DMA_DeInit(&DMA_Handle);
	 }
}