/*
*************************************************************************************************
*文件：otm8009a.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-06-28
*描述：本文件主要是对STM32F469-Dis液晶驱动芯片otm8009a的一些宏定义。
**************************************************************************************************
*/
#ifndef __OTM8009A_H
#define __OTM8009A_H

#ifdef __cplusplus
extern "C" {
#endif /*cpluspluse*/

#include "bsp.h"
  
/************************************OTM8009A方向定义****************************************/

#define OTM8009A_ORIENTATION_PORTRAIT    ((uint32_t)0x00) /* Portrait orientation choice of LCD screen  */
#define OTM8009A_ORIENTATION_LANDSCAPE   ((uint32_t)0x01) /* Landscape orientation choice of LCD screen */

/************************************OTM8009A像素格式定义*************************************/

#define OTM8009A_FORMAT_RGB888    ((uint32_t)0x05) /* Pixel format chosen is RGB888 : 24 bpp */
#define OTM8009A_FORMAT_RBG565    ((uint32_t)0x00) /* Pixel format chosen is RGB565 : 16 bpp */

/************************************LCD有效像素定义*****************************************/

#define OTM8009A_480X800_WIDTH  		((uint32_t)480) /*竖屏模式*/
#define OTM8009A_480X800_HEIGHT 		((uint32_t)800)

#define OTM8009A_800X480_WIDTH  		((uint32_t)800)/*横屏模式*/
#define OTM8009A_800X480_HEIGHT 		((uint32_t)480)

/************************************LCD时序定义********************************************/

#define OTM8009A_480X800_HFP   			((uint32_t)120)  /*竖屏模式*/
#define OTM8009A_480X800_HBP   			((uint32_t)120)
#define OTM8009A_480X800_HSYNC 			((uint32_t)120)
#define OTM8009A_480X800_VFP   			((uint32_t)12)
#define OTM8009A_480X800_VBP   			((uint32_t)12)
#define OTM8009A_480X800_VSYNC 			((uint32_t)12)

/*为什么一样呢，应为竖屏或者是横屏只改变的是有效的像素，对于前沿和后沿来说不变*/
#define OTM8009A_800X400_HFP  			((uint32_t)20)   /*横屏模式*/
#define OTM8009A_800X400_HBP  			((uint32_t)20)
#define OTM8009A_800X400_HSYNC			((uint32_t)2)
#define OTM8009A_800X400_VFP  			((uint32_t)16)
#define OTM8009A_800X400_VBP  			((uint32_t)16)
#define OTM8009A_800X400_VSYNC			((uint32_t)1)

/*************************************OTM8009A常用命令**************************************/

#define  OTM8009A_CMD_NOP                   0x00  /* NOP command      */
#define  OTM8009A_CMD_SWRESET               0x01  /* Sw reset command */
#define  OTM8009A_CMD_RDDMADCTL             0x0B  /* Read Display MADCTR command : read memory display access ctrl */
#define  OTM8009A_CMD_RDDCOLMOD             0x0C  /* Read Display pixel format */
#define  OTM8009A_CMD_SLPIN                 0x10  /* Sleep In command */
#define  OTM8009A_CMD_SLPOUT                0x11  /* Sleep Out command */
#define  OTM8009A_CMD_PTLON                 0x12  /* Partial mode On command */

#define  OTM8009A_CMD_DISPOFF               0x28  /* Display Off command */
#define  OTM8009A_CMD_DISPON                0x29  /* Display On command */

#define  OTM8009A_CMD_CASET                 0x2A  /* Column address set command */
#define  OTM8009A_CMD_PASET                 0x2B  /* Page address set command */

#define  OTM8009A_CMD_RAMWR                 0x2C  /* Memory (GRAM) write command */
#define  OTM8009A_CMD_RAMRD                 0x2E  /* Memory (GRAM) read command  */

#define  OTM8009A_CMD_PLTAR                 0x30  /* Partial area command (4 parameters) */

#define  OTM8009A_CMD_TEOFF                 0x34  /* Tearing Effect Line Off command : command with no parameter */

#define  OTM8009A_CMD_TEEON                 0x35  /* Tearing Effect Line On command : command with 1 parameter 'TELOM' */

/* Parameter TELOM : Tearing Effect Line Output Mode : possible values */
#define OTM8009A_TEEON_TELOM_VBLANKING_INFO_ONLY            0x00
#define OTM8009A_TEEON_TELOM_VBLANKING_AND_HBLANKING_INFO   0x01

#define  OTM8009A_CMD_MADCTR                0x36  /* Memory Access write control command  */

/* Possible used values of MADCTR */
#define OTM8009A_MADCTR_MODE_PORTRAIT       0x00
#define OTM8009A_MADCTR_MODE_LANDSCAPE      0x60  /* MY = 0, MX = 1, MV = 1, ML = 0, RGB = 0 */

#define  OTM8009A_CMD_IDMOFF                0x38  /* Idle mode Off command */
#define  OTM8009A_CMD_IDMON                 0x39  /* Idle mode On command  */

#define  OTM8009A_CMD_COLMOD                0x3A  /* Interface Pixel format command */

/* Possible values of COLMOD parameter corresponding to used pixel formats */
#define  OTM8009A_COLMOD_RGB565             0x55
#define  OTM8009A_COLMOD_RGB888             0x77

#define  OTM8009A_CMD_RAMWRC                0x3C  /* Memory write continue command */
#define  OTM8009A_CMD_RAMRDC                0x3E  /* Memory read continue command  */

#define  OTM8009A_CMD_WRTESCN               0x44  /* Write Tearing Effect Scan line command */
#define  OTM8009A_CMD_RDSCNL                0x45  /* Read  Tearing Effect Scan line command */

/* CABC Management : ie : Content Adaptive Back light Control in IC OTM8009a */
#define  OTM8009A_CMD_WRDISBV               0x51  /* Write Display Brightness command          */
#define  OTM8009A_CMD_WRCTRLD               0x53  /* Write CTRL Display command                */
#define  OTM8009A_CMD_WRCABC                0x55  /* Write Content Adaptive Brightness command */
#define  OTM8009A_CMD_WRCABCMB              0x5E  /* Write CABC Minimum Brightness command     */


/************************************OTM8009A频率定义****************************************/
  
#define OTM8009A_480X800_FREQUENCY_DIVIDER  2   /* LCD Frequency divider      */

/************************************OTM8009A函数申明****************************************/

uint8_t 		OTM8009A_Init    (uint32_t ColorCoding, uint32_t orientation);
extern void DSI_IO_WriteCmd  (uint32_t NbParams, uint8_t *pParams);

#ifdef __cplusplus
	}
#endif /*cplusplus*/

#endif /*__OTM8009A_H*/