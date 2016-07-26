#ifndef __BSP_SD_H
#define __BSP_SD_H

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "bsp.h"

#define MSD_OK                        ((uint8_t)0x00)
#define MSD_ERROR                     ((uint8_t)0x01)
#define MSD_ERROR_SD_NOT_PRESENT      ((uint8_t)0x02)

#define SD_PRESENT               ((uint8_t)0x01)
#define SD_NOT_PRESENT           ((uint8_t)0x00)
#define SD_DATATIMEOUT           ((uint32_t)100000000)
#

#define SD_DMAx_Tx_STREAM                 DMA2_Stream6
#define SD_DMAx_Rx_STREAM                 DMA2_Stream3
#define SD_DMAx_Tx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Rx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Tx_IRQn                   DMA2_Stream6_IRQn
#define SD_DMAx_Rx_IRQn                   DMA2_Stream3_IRQn

uint8_t BSP_SD_Init(void);
uint8_t BSP_SD_DeInit(void);
uint8_t BSP_SD_IsDetected(void);

uint8_t BSP_SD_ReadBlocks(uint32_t *pReadBuffer, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
uint8_t BSP_SD_WriteBlocks(uint32_t *pWriteBuffer, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pReadBuffer, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pWriteBuffer, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumberOfBlocks);
uint8_t BSP_SD_Erase(uint64_t startaddr, uint64_t endaddr);

void BSP_SD_IRQHandler(void);
void BSP_SD_DMA_Tx_IRQHandler(void);
void BSP_SD_DMA_Rx_IRQHandler(void);

HAL_SD_TransferStateTypedef BSP_SD_GetStatus(void);
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypedef *CardInfo);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__BSP_SD_H*/