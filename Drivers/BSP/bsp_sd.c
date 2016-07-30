#include "bsp_sd.h"


SD_HandleTypeDef SD_Handle;
SD_CardInfo CardInfo;

static void BSP_SD_MspInit(SD_HandleTypeDef *hsd);
static void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd);

uint8_t BSP_SD_Init(void)
{
	uint8_t status = MSD_OK;

	SD_Handle.Instance = SDIO;

	SD_Handle.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;							/*数据需要和上升沿保持一致*/
	SD_Handle.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;				 	/*在驱动 SDIO_CK 输出信号前，根据 CLKDIV 值对 SDIOCLK 进行分频*/
	SD_Handle.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
	SD_Handle.Init.BusWide = SDIO_BUS_WIDE_1B;										/*先初始化为1wire的初始化完成后配置为4 wire*/
	SD_Handle.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE; /*开启后防止FIFO溢出*/
	SD_Handle.Init.ClockDiv = SDIO_TRANSFER_CLK_DIV; 								/*最大25MHz的时钟*/

	BSP_SD_MspInit(&SD_Handle);

	if (BSP_SD_IsDetected() != SD_PRESENT)
	{
		return MSD_ERROR_SD_NOT_PRESENT;
	}
	if (HAL_SD_Init(&SD_Handle, &CardInfo)!= SD_OK)
	{
		status = MSD_ERROR;
	}
	if(status == MSD_OK)
	{
		if (HAL_SD_WideBusOperation_Config(&SD_Handle, SDIO_BUS_WIDE_4B) != SD_OK)
		{
			status = MSD_ERROR;
		}
		else
		{
			status = MSD_OK;
		}
	}
	return status;
}
uint8_t BSP_SD_DeInit(void)
{
	uint8_t status = MSD_OK;

	SD_Handle.Instance = SDIO;

	if(HAL_SD_DeInit (&SD_Handle) != HAL_OK)
	{
		status = MSD_ERROR;
	}
	BSP_SD_MspDeInit(&SD_Handle);

	return status;

}
uint8_t BSP_SD_ReadBlocks(uint32_t *pReadBuffer, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumberOfBlocks)
{
	if(HAL_SD_ReadBlocks(&SD_Handle, pReadBuffer, ReadAddr, BlockSize, NumberOfBlocks) != SD_OK)
	{
		return MSD_ERROR;
	}
	else
	{
		return MSD_OK;
	}
}
uint8_t BSP_SD_WriteBlocks(uint32_t *pWriteBuffer, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumberOfBlocks)
{

	if(HAL_SD_WriteBlocks(&SD_Handle, pWriteBuffer, WriteAddr, BlockSize, NumberOfBlocks) != SD_OK)
	{
		return MSD_ERROR;
	}
	else
	{
		return MSD_OK;
	}
}
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pReadBuffer, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumberOfBlocks)
{
		uint8_t status = MSD_OK;

	if(HAL_SD_ReadBlocks_DMA(&SD_Handle, pReadBuffer, ReadAddr, BlockSize, NumberOfBlocks) != SD_OK)
	{
		status = MSD_ERROR;
	}
	if (status == MSD_OK)
	{
		if (HAL_SD_CheckReadOperation(&SD_Handle, (uint32_t)SD_DATATIMEOUT) != SD_OK)
		{
			status = MSD_ERROR;
		}
		else
		{
			status = MSD_OK;
		}
	}
	return status;
}
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pWriteBuffer, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumberOfBlocks)
{
	uint8_t status = MSD_OK;
	if(HAL_SD_WriteBlocks_DMA(&SD_Handle, pWriteBuffer, WriteAddr, BlockSize,  NumberOfBlocks) != SD_OK)
	{
		status = MSD_ERROR;
	}
	if (status == MSD_OK)
	{
		if (HAL_SD_CheckWriteOperation(&SD_Handle, (uint32_t)SD_DATATIMEOUT) != SD_OK)
		{
			status = MSD_ERROR;
		}
		else
		{
			status = MSD_OK;
		}
	}
	return status;

}
uint8_t BSP_SD_Erase(uint64_t startaddr, uint64_t endaddr)
{
	if(HAL_SD_Erase(&SD_Handle, startaddr, endaddr) != SD_OK)
	{
		return MSD_ERROR;
	}
	else
	{
		return MSD_OK;
	}
}

uint8_t BSP_SD_IsDetected(void)
{
	if(HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_2) == GPIO_PIN_SET)
	{
		return SD_NOT_PRESENT;
	}
	else
	{
		return SD_PRESENT;
	}
}
static void BSP_SD_MspInit(SD_HandleTypeDef *hsd)
{
	GPIO_InitTypeDef GPIO_InitStru;
	DMA_HandleTypeDef DMA_RX_Handle;
	DMA_HandleTypeDef DMA_TX_Handle;

	/*使能SDIO的时钟*/
	__HAL_RCC_SDIO_CLK_ENABLE();

	/*使能SDIO GPIO时钟*/
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	 /*使能DMA2的时钟*/
	 __HAL_RCC_DMA2_CLK_ENABLE();

	 /*配置SDIO卡是否接入*/
	 GPIO_InitStru.Pin = GPIO_PIN_2;
	 GPIO_InitStru.Mode = GPIO_MODE_INPUT;
	 GPIO_InitStru.Speed = GPIO_SPEED_HIGH;
	 GPIO_InitStru.Pull = GPIO_PULLUP;
	 GPIO_InitStru.Alternate = GPIO_AF12_SDIO;

	 HAL_GPIO_Init(GPIOG, &GPIO_InitStru);

	 /*配置SDIO卡GPIO(数据，时钟)*/
	 GPIO_InitStru.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
	 GPIO_InitStru.Mode = GPIO_MODE_AF_PP;
	 GPIO_InitStru.Speed = GPIO_SPEED_HIGH;
	 GPIO_InitStru.Pull = GPIO_PULLUP;
	 GPIO_InitStru.Alternate = GPIO_AF12_SDIO;

	 HAL_GPIO_Init(GPIOC, &GPIO_InitStru);

	 GPIO_InitStru.Pin =GPIO_PIN_2;

	 HAL_GPIO_Init(GPIOD, &GPIO_InitStru);

	 /*设置SDIO中断优先级*/
	HAL_NVIC_SetPriority(SDIO_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(SDIO_IRQn);

	/*配置SDIO DMA read*/
	DMA_RX_Handle.Instance = SD_DMAx_Rx_STREAM;
	DMA_RX_Handle.Init.Channel  = SD_DMAx_Rx_CHANNEL; 
	DMA_RX_Handle.Init.Direction = DMA_PERIPH_TO_MEMORY;            
	DMA_RX_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
	DMA_RX_Handle.Init.MemInc = DMA_MINC_ENABLE;
	DMA_RX_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;  
	DMA_RX_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;    
	DMA_RX_Handle.Init.Mode = DMA_PFCTRL;
	DMA_RX_Handle.Init.Priority = DMA_PRIORITY_VERY_HIGH;             
	DMA_RX_Handle.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	/*FIFO的阈值和突然模式需要匹配，根据参考手册配置*/
	DMA_RX_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	DMA_RX_Handle.Init.MemBurst = DMA_MBURST_INC4;             
	DMA_RX_Handle.Init.PeriphBurst = DMA_PBURST_INC4;   

	/*链接DMA和外设*/
	__HAL_LINKDMA(hsd, hdmarx, DMA_RX_Handle);
	/*初始化DMA read 数据流*/
	HAL_DMA_DeInit(&DMA_RX_Handle);
	HAL_DMA_Init(&DMA_RX_Handle);

	/*配置SDIO DMA read*/
	DMA_TX_Handle.Instance = SD_DMAx_Tx_STREAM;
	DMA_TX_Handle.Init.Channel  = SD_DMAx_Tx_CHANNEL; 
	DMA_TX_Handle.Init.Direction = DMA_MEMORY_TO_PERIPH;            
	DMA_TX_Handle.Init.PeriphInc = DMA_PINC_DISABLE;
	DMA_TX_Handle.Init.MemInc = DMA_MINC_ENABLE;
	DMA_TX_Handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	DMA_TX_Handle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	DMA_TX_Handle.Init.Mode = DMA_PFCTRL;
	DMA_TX_Handle.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	DMA_TX_Handle.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	/*FIFO的阈值和突然模式需要匹配，根据参考手册配置*/
	DMA_TX_Handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	DMA_TX_Handle.Init.MemBurst = DMA_MBURST_INC4;             
	DMA_TX_Handle.Init.PeriphBurst = DMA_PBURST_INC4;   

	/*链接DMA和外设*/
	__HAL_LINKDMA(hsd, hdmatx, DMA_TX_Handle);
	/*初始化DMA read 数据流*/
	HAL_DMA_DeInit(&DMA_TX_Handle);
	HAL_DMA_Init(&DMA_TX_Handle); 

	/*配置SDIO DMA写中断*/
  HAL_NVIC_SetPriority(SD_DMAx_Rx_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(SD_DMAx_Rx_IRQn);

  /*配置SDIO DMA读中断*/
  HAL_NVIC_SetPriority(SD_DMAx_Tx_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(SD_DMAx_Tx_IRQn);      
}
static void BSP_SD_MspDeInit(SD_HandleTypeDef *hsd)
{
	DMA_HandleTypeDef DMA_RX_Handle;
	DMA_HandleTypeDef DMA_TX_Handle;

	/*禁止SDIO  DMA 读写中断*/
    HAL_NVIC_DisableIRQ(SD_DMAx_Rx_IRQn);
    HAL_NVIC_DisableIRQ(SD_DMAx_Tx_IRQn);

    /*恢复DMA数据流*/
    DMA_RX_Handle.Instance = SD_DMAx_Rx_STREAM;
    HAL_DMA_DeInit(&DMA_RX_Handle);

    /*恢复DMA数据流*/
    DMA_TX_Handle.Instance = SD_DMAx_Tx_STREAM;
    HAL_DMA_DeInit(&DMA_TX_Handle);

    /*禁止SDIO中断*/
    HAL_NVIC_DisableIRQ(SDIO_IRQn);

    /* DeInit GPIO pins can be done in the application
       (by surcharging this __weak function) */

    /*禁止SDIO时钟*/
    __HAL_RCC_SDIO_CLK_DISABLE();
}
void BSP_SD_IRQHandler(void)
{
	HAL_SD_IRQHandler(&SD_Handle);
}
void BSP_SD_DMA_Tx_IRQHandler(void)
{
	/*
	*这种方式也是可以的，通过DMA link函数将SD_Handle和DMA链接在一起，
	*访问SD_Handle.hdmatx既可以访问DMA,也可以直接将DMA_Handel声明为全局结构体
	*/
  HAL_DMA_IRQHandler(SD_Handle.hdmatx);
}

/**
  * @brief  Handles SD DMA Rx transfer interrupt request.
  */
void BSP_SD_DMA_Rx_IRQHandler(void)
{
  HAL_DMA_IRQHandler(SD_Handle.hdmarx);
}
 
HAL_SD_TransferStateTypedef BSP_SD_GetStatus(void)
{
  return(HAL_SD_GetStatus(&SD_Handle));
}

/**
  * @brief  Get SD information about specific SD card.
  * @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
  */
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypedef *CardInfo)
{
  /* Get SD card Information */
  HAL_SD_Get_CardInfo(&SD_Handle, CardInfo);
}