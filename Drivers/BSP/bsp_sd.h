/*
*************************************************************************************************
*文件：bsp_sd.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-08-01
*描述：STM32F469NI-Discovery开发套件SD卡驱动程序。
**************************************************************************************************
*/
#ifndef __BSP_SD_H
#define __BSP_SD_H

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "bsp.h"

/****************************************SD卡结构体定义****************************************************/
#define SD_CardInfo HAL_SD_CardInfoTypedef

/***************************************SD卡状态定义*******************************************************/
#define MSD_OK                        ((uint8_t)0x00)
#define MSD_ERROR                     ((uint8_t)0x01)
#define MSD_ERROR_SD_NOT_PRESENT      ((uint8_t)0x02)

#define SD_PRESENT                    ((uint8_t)0x01)
#define SD_NOT_PRESENT                ((uint8_t)0x00)
#define SD_DATATIMEOUT                ((uint32_t)100000000)

/*****************************************SD卡DMA相关定义***************************************************/
#define SD_DMAx_Tx_STREAM                 DMA2_Stream6
#define SD_DMAx_Rx_STREAM                 DMA2_Stream3
#define SD_DMAx_Tx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Rx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Tx_IRQn                   DMA2_Stream6_IRQn
#define SD_DMAx_Rx_IRQn                   DMA2_Stream3_IRQn
#define SD_DMAx_Tx_IRQHandler             DMA2_Stream6_IRQHandler
#define SD_DMAx_Rx_IRQHandler             DMA2_Stream3_IRQHandler

/**********************************************函数声明*******************************************************/
uint8_t BSP_SD_Init(void);
uint8_t BSP_SD_DeInit(void);
uint8_t BSP_SD_IsDetected(void);

uint8_t BSP_SD_ReadBlocks(uint32_t *pReadBuffer, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
uint8_t BSP_SD_WriteBlocks(uint32_t *pWriteBuffer, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pReadBuffer, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pWriteBuffer, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
uint8_t BSP_SD_Erase(uint64_t startaddr, uint64_t endaddr);

void SDIO_IRQHandler(void);
void SD_DMAx_Tx_IRQHandler(void);
void SD_DMAx_Rx_IRQHandler(void);

HAL_SD_TransferStateTypedef BSP_SD_GetStatus(void);
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypedef *CardInfo);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__BSP_SD_H*/