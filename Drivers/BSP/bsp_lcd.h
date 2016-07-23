/*
*************************************************************************************************
*文件：bsp_lcd.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-06-01
*描述：STM32F469NI-Discovery开发套件4"TFT液晶驱动相关申明。
**************************************************************************************************
*/
#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#ifdef __cplusplus
extern "C" {
#endif /*cpluspluse*/

#include "bsp.h"
#include "otm8009a.h"
#include "./Font/fonts.h"
/*
****************************************************************************************************
*												LCD显示枚举定义
* **************************************************************************************************
 */ 
typedef enum
{
	LCD_ORIENTATION_PORTRAIT  = 0x00, /*竖屏显示模式*/
	LCD_ORIENTATION_LANDSCAPE = 0x01, /*横屏显示模式*/
	LCD_ORIENTATION_INVALID   = 0x02  /*无效显示模式*/
}LCD_OrientationTypeDef;

typedef struct
{
  uint32_t TextColor; /*!< Specifies the color of text */
  uint32_t BackColor; /*!< Specifies the background color below the text */
  sFONT    *pFont;    /*!< Specifies the font used for the text */

} LCD_DrawPropTypeDef;

typedef struct
{
  int16_t X; /*!< geometric X position of drawing */
  int16_t Y; /*!< geometric Y position of drawing */

} Point;
typedef Point *pPoint;
typedef enum
{
  CENTER_MODE             = 0x01,    /*居中模式*/
  RIGHT_MODE              = 0x02,    /*右对齐*/
  LEFT_MODE               = 0x03     /*左对齐*/

}Text_AlignModeTypdef;
/*
****************************************************************************************************
*												LTDC接口相关定义
* **************************************************************************************************
 */   
#define LCD_OTM8009A_ID                   ((uint32_t) 0)                    /*LCD虚拟通道*/
#define LTDC_MAX_LAYER_NUMBER             ((uint8_t)2)                      /*LTDC最大画面层数*/
#define LCD_FB_START_ADDRESS              ((uint32_t)0xC0000000)            /*像素缓存起始地址*/
#define LTDC_ACTIVE_LAYER_BACKGROUND      ((uint32_t) 0)                    /*背景层定义*/
#define LTDC_ACTIVE_LAYER_FOREGROUND      ((uint32_t) 1)                    /*前景层定义*/
#define LTDC_NB_OF_LAYERS                 ((uint32_t) 2)                    /*LTDC画面层数*/


#define LTDC_DEFAULT_ACTIVE_LAYER         LTDC_ACTIVE_LAYER_FOREGROUND      /*LTDC默认激活的层*/
#define LCD_LayerCfgTypeDef               LTDC_LayerCfgTypeDef
#define LCD_DEFAULT_FONT                  Font24                            /*定义默认的字体*/

   
#define   LCD_OK         0x00
#define   LCD_ERROR      0x01
#define   LCD_TIMEOUT    0x02

/***********************************LCD颜色定义ARGB8888格式****************************************/

#define LCD_COLOR_BLUE          ((uint32_t) 0xFF0000FF)     /*蓝色*/
#define LCD_COLOR_GREEN         ((uint32_t) 0xFF00FF00)     /*绿色*/
#define LCD_COLOR_RED           ((uint32_t) 0xFFFF0000)     /*红色*/
#define LCD_COLOR_CYAN          ((uint32_t) 0xFF00FFFF)     /*蓝绿色*/
#define LCD_COLOR_MAGENTA       ((uint32_t) 0xFFFF00FF)     /*品红*/
#define LCD_COLOR_YELLOW        ((uint32_t) 0xFFFFFF00)     /*黄色*/
#define LCD_COLOR_WHITE         ((uint32_t) 0xFFFFFFFF)     /*白色*/
#define LCD_COLOR_GRAY          ((uint32_t) 0xFF808080)     /*灰色*/
#define LCD_COLOR_BLACK         ((uint32_t) 0xFF000000)     /*黑色*/
#define LCD_COLOR_BROWN         ((uint32_t) 0xFFA52A2A)     /*棕色*/
#define LCD_COLOR_ORANGE        ((uint32_t) 0xFFFFA500)     /*橙色*/
#define LCD_COLOR_TRANSPARENT   ((uint32_t) 0xFF000000)     /*透明的*/

#define LCD_COLOR_LIGHTBLUE     ((uint32_t) 0xFF8080FF)     /*浅蓝*/
#define LCD_COLOR_LIGHTGREEN    ((uint32_t) 0xFF80FF80)     /*浅绿*/
#define LCD_COLOR_LIGHTRED      ((uint32_t) 0xFFFF8080)     /*浅红*/
#define LCD_COLOR_LIGHTCYAN     ((uint32_t) 0xFF80FFFF)     /*淡青色*/
#define LCD_COLOR_LIGHTMAGENTA  ((uint32_t) 0xFFFF80FF)     /*淡品红*/
#define LCD_COLOR_LIGHTYELLOW   ((uint32_t) 0xFFFFFF80)     /*淡黄色*/
#define LCD_COLOR_LIGHTGRAY     ((uint32_t) 0xFFD3D3D3)     /*浅灰色*/

#define LCD_COLOR_DARKBLUE      ((uint32_t) 0xFF000080)     /*深蓝色*/
#define LCD_COLOR_DARKGREEN     ((uint32_t) 0xFF008000)     /*深绿色*/
#define LCD_COLOR_DARKRED       ((uint32_t) 0xFF800000)     /*深红色*/
#define LCD_COLOR_DARKCYAN      ((uint32_t) 0xFF008080)     /*深蓝绿色*/
#define LCD_COLOR_DARKMAGENTA   ((uint32_t) 0xFF800080)     /*深品红*/
#define LCD_COLOR_DARKYELLOW    ((uint32_t) 0xFF808000)     /*深黄色*/
#define LCD_COLOR_DARKGRAY      ((uint32_t) 0xFF404040)     /*深灰色*/

/*
*****************************************************************************************************
*													函数声明
*****************************************************************************************************
*/
uint8_t  BSP_LCD_Init(LCD_OrientationTypeDef orientation);
void 		 BSP_LCD_Reset(void);
void 		 BSP_LCD_MspInit(void);
void     BSP_LCD_Clear(uint32_t Color);
void     BSP_LCD_ClearStringLine(uint32_t Line);
void     BSP_LCD_DisplayOn(void);
void     BSP_LCD_DisplayOff(void);

void 		 DSI_IO_WriteCmd(uint32_t NbParams, uint8_t *pParams);
void 		 BSP_LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address);
void 		 BSP_LCD_SelectLayer(uint32_t LayerIndex);
void     BSP_LCD_SetXSize(uint16_t imageWidthPixels);
void     BSP_LCD_SetYSize(uint16_t imageHeighPixels);
uint32_t BSP_LCD_GetXSize(void);
uint32_t BSP_LCD_GetYSize(void);
void 		 BSP_LCD_SetFont(sFONT *fonts);
sFONT    *BSP_LCD_GetFont(void);
uint32_t BSP_LCD_GetBackColor(void);
void     BSP_LCD_SetBackColor(uint32_t Color);
uint32_t BSP_LCD_GetTextColor(void);
void     BSP_LCD_SetTextColor(uint32_t Color);
void     BSP_LCD_ResetColorKeying(uint32_t LayerIndex);
void     BSP_LCD_SetColorKeying(uint32_t LayerIndex, uint32_t RGBValue);
void     BSP_LCD_SetLayerWindow(uint32_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     BSP_LCD_SetLayerAddress(uint32_t LayerIndex, uint32_t Address);
void     BSP_LCD_SetTransparency(uint32_t LayerIndex, uint32_t Transparency);
void     BSP_LCD_SelectLayerVisible(uint32_t LayerIndex, FunctionalState State);
void     BSP_LCD_SelectLayer(uint32_t LayerIndex);

uint32_t BSP_LCD_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void    BSP_LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGBValue);
void    BSP_LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);
void 		BSP_LCD_DsiplayStringAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Text_AlignModeTypdef Mode);
void 		BSP_LCD_DisplayStringAtLine(uint16_t Line, uint8_t *pstr);

void 		BSP_LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void 		BSP_LCD_DrawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void 		BSP_LCD_DrawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void 		BSP_LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void 		BSP_LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void 		BSP_LCD_DrawEllipse(uint16_t Xpos, uint16_t Ypos, uint16_t XRadius, uint16_t YRadius);
void 		BSP_LCD_DrawPolygon(pPoint Points, uint16_t PointCount);
void 		BSP_DrawBitmap(uint16_t Xpos, uint16_t Ypos, const uint8_t *pbmp);

void    BSP_LCD_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void 		BSP_LCD_FillCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void    BSP_LCD_FillEllipse(uint16_t Xpos, uint16_t Ypos, uint16_t XRadius, uint16_t YRadius);
void    BSP_LCD_FillPolygon(pPoint Points, uint16_t PointCount);
void FillTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3);

__weak void LCD_LTDC_ER_IRQHandler(void);
__weak void LCD_DSI_IRQHandler(void);
__weak void LCD_LTDC_IRQHandler(void);
__weak void LCD_DMA2D_IRQHandler(void);

#ifdef __cplusplus
}
#endif/*cplusplus*/

#endif/*__BSP_LCD_H*/
