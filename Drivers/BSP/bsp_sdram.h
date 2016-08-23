/*
*************************************************************************************************
*文件：bsp_sdarm.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-06-04
*描述：STM32F469NI-Discovery开发套件SDARN驱动程序。
**************************************************************************************************
*/
#ifndef __BSP_SDRAM_H
#define __BSP_SDRAM_H

#ifdef __cplusplus
extern "C" {
#endif	/*__cplusplus*/

#include "stm32f4xx_hal.h"
 
/*
**************************************************************************************************
*										SDRAM状态定义
**************************************************************************************************
 */ 
#define SDRAM_OK            ((uint8_t)0x00)
#define SDRAM_ERROR         ((uint8_t)0x01)
 /*
**************************************************************************************************
*										SDRAM地址和大小定义
**************************************************************************************************
 */
#define SDRAM_DEVICE_ADDR   ((uint32_t)0xC0000000)
#define SDRAM_BANK1_ADDR    ((uint32_t)0xC0000000)
#define SDRAM_BANK2_ADDR    ((uint32_t)0xC0400000)
#define SDRAM_BANK3_ADDR    ((uint32_t)0xC0800000)
#define SDRAM_BANK4_ADDR    ((uint32_t)0xC0C00000)
#define SDRAM_DEVICE_SIZE   ((uint32_t)0x1000000)
/*
**************************************************************************************************
*										SDRAM超时和刷新频率定义
**************************************************************************************************
 */
#define SDRAM_TIME_OUT 		 ((uint32_t)0XFFFF)
#define REFRESH_COUNT 		 ((uint32_t)0X0569)      /*刷新频率根据90Mhz时钟计算*/
/*
**************************************************************************************************
*										SDRAM DAM通道和中断定义
**************************************************************************************************
 */
#define SDRAM_DMA_CHANNEL		DMA_CHANNEL_0 
#define SDRAM_DMA_STREAM		DMA2_Stream0
#define SDRAM_DMA_IRQn			DMA2_Stream0_IRQn
#define SDRAM_DMA_IRQHandler	DMA2_Stream0_IRQHandler
/*
**************************************************************************************************
*										SDRAM模式寄存器参数定义
**************************************************************************************************
 */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_1              ((uint16_t)0x0010)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)
/*
**************************************************************************************************
*										SDRAM驱动外部函申明
**************************************************************************************************
 */ 
uint8_t BSP_SDRAM_Init(void);
uint8_t BSP_SDRAM_DeInit(void);
uint8_t BSP_SDRAM_ReadData( uint32_t pAddress, uint32_t *pDstBuffer, uint32_t BufferSize);
uint8_t BSP_SDRAM_ReadDataDMA( uint32_t pAddress, uint32_t *pDstBuffer, uint32_t BufferSize);
uint8_t BSP_SDRAM_WriteData(uint32_t pAddress, uint32_t *pDstBuffer, uint32_t BufferSize);
uint8_t BSP_SDRAM_WriteDataDMA(uint32_t pAddress, uint32_t *pDstBuffer, uint32_t BufferSize);
uint8_t BSP_SDRAM_SendCmd(FMC_SDRAM_CommandTypeDef *SdramCmd);

void BSP_SDRAM_MspDeInit(SDRAM_HandleTypeDef *HSDRAM );
void SDRAM_DMA_IRQHandler(void);

#ifdef __cplusplus
}
#endif/*cplusplus*/

#endif /*__BSP_SDARM*/