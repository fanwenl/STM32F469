/*********************************************************************
*          Portions COPYRIGHT 2015 STMicroelectronics                *
*          Portions SEGGER Microcontroller GmbH & Co. KG             *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.28 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The  software has  been licensed  to STMicroelectronics International
N.V. a Dutch company with a Swiss branch and its headquarters in Plan-
les-Ouates, Geneva, 39 Chemin du Champ des Filles, Switzerland for the
purposes of creating libraries for ARM Cortex-M-based 32-bit microcon_
troller products commercialized by Licensee only, sublicensed and dis_
tributed under the terms and conditions of the End User License Agree_
ment supplied by STMicroelectronics International N.V.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

/**
  ******************************************************************************
  * @attention
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#include "GUI.h"
#include "GUIDRV_Lin.h"
#include "LCDConf.h"

/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/
//
// Physical display size
//
#define XSIZE_PHYS 480
#define YSIZE_PHYS 800

#undef  LCD_SWAP_XY
#undef  LCD_MIRROR_Y

#define LCD_SWAP_XY  1 
#define LCD_MIRROR_Y 1
//
// Color conversion
// Display driver

#define COLOR_CONVERSION_0 GUICC_M8888I
#define DISPLAY_DRIVER_0   GUIDRV_LIN_32


#if (GUI_NUM_LAYERS > 1)
#define COLOR_CONVERSION_1 GUICC_M8888I
#define DISPLAY_DRIVER_1   GUIDRV_LIN_32
#endif

//
// Buffers / VScreens
//
#define NUM_BUFFERS  3 // Number of multiple buffers to be used
#define NUM_VSCREENS 1 // Number of virtual screens to be used

#define GUI_NUM_LAYERS 1

#define LCD_SWAP_XY 1


/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   VRAM_ADDR
  #define VRAM_ADDR 0xC0000000 // TBD by customer: This has to be the frame buffer start address
#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION_0
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER_0
  #error No display driver defined!
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens and multiple buffers are not allowed!
#endif
#ifndef GUI_NUM_LAYERS
  #error No GUI_NUM_LAYERS defined！
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

#define LCD_LAYER0_FRAME_BUFFER  ((uint32_t)0xC0000000)  
#define LCD_LAYER1_FRAME_BUFFER  ((uint32_t)0xC0500000)  


LCD_LayerPropTypedef     layer_prop[GUI_NUM_LAYERS];
volatile int32_t LCD_ActiveRegion    = 1;
volatile int32_t LCD_Refershing      = 0;



static const LCD_API_COLOR_CONV * apColorConvAPI[] = 
{
  COLOR_CONVERSION_0,
#if GUI_NUM_LAYERS > 1
  COLOR_CONVERSION_1,
#endif
};

#if GUI_NUM_LAYERS < 2
U32 LCD_Addr[GUI_NUM_LAYERS] = {LCD_LAYER0_FRAME_BUFFER};
#else
U32 LCD_Addr[GUI_NUM_LAYERS] = {LCD_LAYER0_FRAME_BUFFER, LCD_LAYER1_FRAME_BUFFER};
#endif


static void     DMA2D_CopyBufferWithAlpha(U32 LayerIndex, void * pSrc, void * pDst, U32 xSize, U32 ySize, U32 OffLineSrc, U32 OffLineDst);
static void     DMA2D_CopyBuffer(U32 LayerIndex, void * pSrc, void * pDst, U32 xSize, U32 ySize, U32 OffLineSrc, U32 OffLineDst);
static void     DMA2D_FillBuffer(U32 LayerIndex, void * pDst, U32 xSize, U32 ySize, U32 OffLine, U32 ColorIndex);

static void     LCD_LL_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst);
static void     LCD_LL_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize);
static void     LCD_LL_FillRect(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex);
static void     LCD_LL_DrawBitmap8bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine);
static void     LCD_LL_DrawBitmap16bpp(int LayerIndex, int x, int y, U16 const * p, int xSize, int ySize, int BytesPerLine);
static void     LCD_LL_DrawBitmap32bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine);

/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*   
*/
void LCD_X_Config(void) {

  U32 i;

  //初始化LCD
  BSP_LCD_Init(LCD_ORIENTATION_PORTRAIT);

  /*设置显示层缓存*/
#if (NUM_BUFFERS > 1)
	for(i = 0; i < GUI_NUM_LAYERS; i++)
	{
	 GUI_MULTIBUF_ConfigEx(i,NUM_BUFFERS);
	}
#endif
 
  /*设置显示驱动，系那是颜色格式*/
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_0, COLOR_CONVERSION_0, 0, 0);

	if (LCD_GetSwapXYEx(0)) 
	{
	  LCD_SetSizeEx (0, YSIZE_PHYS, XSIZE_PHYS);
	  LCD_SetVSizeEx(0, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
	} 
	else 
	{
	  LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
	  LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
	}

   /*设置液晶显示缓存地址*/ 
   layer_prop[0].address = LCD_LAYER0_FRAME_BUFFER;

  #if(GUI_NUM_LAYERS > 1)
    GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_1, COLOR_CONVERSION_1, 0, 1);

            /*设置层的显示区域*/
    if (LCD_GetSwapXYEx(1)) 
    {
        LCD_SetSizeEx (1, YSIZE_PHYS, XSIZE_PHYS);
        LCD_SetVSizeEx(1, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
      } 
      else 
      {
        LCD_SetSizeEx (1, XSIZE_PHYS, YSIZE_PHYS);
        LCD_SetVSizeEx(1, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
      }
   layer_prop[1].address = LCD_LAYER1_FRAME_BUFFER; 
  #endif

  /*链接显示驱动，设置GRAM的地址*/
  for(i = 0; i < GUI_NUM_LAYERS; i++)
  {

    layer_prop[i].pColorConvAPI = (LCD_API_COLOR_CONV *)apColorConvAPI[i];

    layer_prop[i].pending_buffer = -1;

    
   LCD_SetVRAMAddrEx(i, (void *)(layer_prop[i].address));


    /*读取颜色的深度，并将其左移三位装换为字节*/
    layer_prop[i].BytesPerPixel = LCD_GetBitsPerPixelEx(i) >> 3;

    LCD_SetDevFunc(i, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))LCD_LL_CopyBuffer);
    LCD_SetDevFunc(i, LCD_DEVFUNC_COPYRECT,   (void(*)(void))LCD_LL_CopyRect);

    LCD_SetDevFunc(i, LCD_DEVFUNC_FILLRECT, (void(*)(void))LCD_LL_FillRect);
    LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_8BPP, (void(*)(void))LCD_LL_DrawBitmap8bpp); 
    LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_16BPP, (void(*)(void))LCD_LL_DrawBitmap16bpp);
    LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_32BPP, (void(*)(void))LCD_LL_DrawBitmap32bpp);

  }

    BSP_LCD_LayerDefaultInit(0, LCD_LAYER0_FRAME_BUFFER);
  #if (GUI_NUM_LAYERS > 1)    
    BSP_LCD_LayerDefaultInit(1, LCD_LAYER1_FRAME_BUFFER);
  #endif
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if 
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/

int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;
  int xPos, yPos;
  U32 addr,Color;

  switch (Cmd) {
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
//    BSP_LCD_LayerDefaultInit(LayerIndex, layer_prop[LayerIndex].address);
    break;
  }
  case LCD_X_SETVRAMADDR: {
    //
    // Required for setting the address of the video RAM for drivers
    // with memory mapped video RAM which is passed in the 'pVRAM' element of p
    //
    LCD_X_SETVRAMADDR_INFO * p;
    p = (LCD_X_SETVRAMADDR_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SETORG: {
    //
    // Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
    // 设置LCD的原点
    addr = layer_prop[LayerIndex].address + ((LCD_X_SETORG_INFO *)pData)->yPos * layer_prop[LayerIndex].xSize * layer_prop[LayerIndex].BytesPerPixel;
    HAL_LTDC_SetAddress(&LTDC_Handle, addr, LayerIndex);
    break;
  }
    case LCD_X_SETPOS:
      //设置LCD窗口的原点
    HAL_LTDC_SetWindowPosition(&LTDC_Handle,
                           ((LCD_X_SETPOS_INFO *)pData)->xPos, 
                           ((LCD_X_SETPOS_INFO *)pData)->yPos, 
                           LayerIndex);
    break;

  
  case LCD_X_SHOWBUFFER: {
    //
    // Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
    //
    LCD_X_SHOWBUFFER_INFO * p;
    p = (LCD_X_SHOWBUFFER_INFO *)pData;
    layer_prop[LayerIndex].pending_buffer = p->Index;
    break;
  }
  case LCD_X_SETLUTENTRY: {
    //
    // Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
    //
    LCD_X_SETLUTENTRY_INFO * p;
    p = (LCD_X_SETLUTENTRY_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_ON: {
    //
    // Required if the display controller should support switching on and off
    //
    BSP_LCD_DisplayOn();
    break;
  }
  case LCD_X_OFF: {
    //
    // Required if the display controller should support switching on and off
    //
    BSP_LCD_DisplayOff();
    break;
  }
  case LCD_X_SETSIZE:
      GUI_GetLayerPosEx(LayerIndex, &xPos, &yPos);
      layer_prop[LayerIndex].xSize = ((LCD_X_SETSIZE_INFO *)pData)->xSize;
      layer_prop[LayerIndex].ySize = ((LCD_X_SETSIZE_INFO *)pData)->ySize;
      HAL_LTDC_SetWindowPosition(&LTDC_Handle, xPos, yPos, LayerIndex);
      break;
     

   case LCD_X_SETVIS:
       if(((LCD_X_SETVIS_INFO *)pData)->OnOff  == ENABLE )
    {
      /*使能LTDC层显示，操作LTDC之前需要关闭DSI*/
      __HAL_DSI_WRAPPER_DISABLE(&DSI_Handle);
      __HAL_LTDC_LAYER_ENABLE(&LTDC_Handle, LayerIndex); 
      __HAL_DSI_WRAPPER_ENABLE(&DSI_Handle);
    }
    else
    {
      /*关闭LTDC显示*/
      __HAL_DSI_WRAPPER_DISABLE(&DSI_Handle);
      __HAL_LTDC_LAYER_DISABLE(&LTDC_Handle, LayerIndex); 
      __HAL_DSI_WRAPPER_ENABLE(&DSI_Handle);
    }
    /*重新加载LTDC配置*/
    __HAL_LTDC_RELOAD_CONFIG(&LTDC_Handle); 
    /*更新DSI配置*/
    HAL_DSI_Refresh(&DSI_Handle); 
    break;
   case LCD_X_SETALPHA:
   HAL_LTDC_SetAlpha(&LTDC_Handle, ((LCD_X_SETALPHA_INFO *)pData)->Alpha, LayerIndex);
    break;
   case LCD_X_SETALPHAMODE:
   return 0;
   case LCD_X_SETCHROMAMODE:
       if(((LCD_X_SETCHROMAMODE_INFO *)pData)->ChromaMode != 0)
    {
      HAL_LTDC_EnableColorKeying(&LTDC_Handle, LayerIndex);
    }
    else
    {
      HAL_LTDC_DisableColorKeying(&LTDC_Handle, LayerIndex);      
    }
    break;

   case LCD_X_SETCHROMA: /*设置色彩浓度*/
        Color = ((((LCD_X_SETCHROMA_INFO *)pData)->ChromaMin & 0xFF0000) >> 16) |\
             (((LCD_X_SETCHROMA_INFO *)pData)->ChromaMin & 0x00FF00) |\
            ((((LCD_X_SETCHROMA_INFO *)pData)->ChromaMin & 0x0000FF) << 16);
    
    HAL_LTDC_ConfigColorKeying(&LTDC_Handle, Color, LayerIndex);
    break;
  default:
    r = -1;
  }
  return r;
}
/**
 * [GetPixelformat 获取像素的RGB格式]
 * @param  LayerIndex [显示层编号]
 * @return            [颜色的格式]
 */
static U32 GetPixelformat(U32 LayerIndex)
{
  const LCD_API_COLOR_CONV * pColorConvAPI;

  pColorConvAPI = layer_prop[LayerIndex].pColorConvAPI;
  
  if (pColorConvAPI == GUICC_M8888I) 
  {
    return LTDC_PIXEL_FORMAT_ARGB8888;
  } 
  else if (pColorConvAPI == GUICC_M888) 
  {
    return LTDC_PIXEL_FORMAT_RGB888;
  } 
  else if (pColorConvAPI == GUICC_M565) 
  {
    return LTDC_PIXEL_FORMAT_RGB565;
  } 
  return 0;
}
/**
 * [DMA2D_CopyBuffer 复制一个缓存区]
 * @param LayerIndex [显示层索引]
 * @param pSrc       [原地址]
 * @param pDst       [目地址]
 * @param xSize      [x方向的大小]
 * @param ySize      [y方向的大小]
 * @param OffLineSrc [源偏移地址]
 * @param OffLineDst [目偏移地址]
 */
static void DMA2D_CopyBuffer(U32 LayerIndex, void * pSrc, void * pDst, U32 xSize, U32 ySize, U32 OffLineSrc, U32 OffLineDst)
{
  U32 PixelFormat;

  PixelFormat = GetPixelformat(LayerIndex);
  /*M2M模式，最后一行传输完成中断模式*/
  DMA2D->CR      = 0x00000000UL | (1 << 9);  

  /* Set up pointers */
 // if(LayerIndex == 1)   /*前景层*/
 // {
    DMA2D->FGMAR   = (U32)pSrc;                       
    DMA2D->OMAR    = (U32)pDst;                       
    DMA2D->FGOR    = OffLineSrc;                      
    DMA2D->OOR     = OffLineDst; 

    /* Set up pixel format */  
    DMA2D->FGPFCCR = PixelFormat;  
  //}
  // else                /*背景层*/
  // {
  //   DMA2D->BGMAR   = (U32)pSrc;                       
  //   DMA2D->OMAR    = (U32)pDst;                       
  //   DMA2D->BGOR    = OffLineSrc;                      
  //   DMA2D->OOR     = OffLineDst; 

  // /* Set up pixel format */  
  //   DMA2D->BGPFCCR = PixelFormat;
  // }
  /*  Set up size */
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize; 

  DMA2D->CR     |= DMA2D_CR_START;   

  /* Wait until transfer is done */
  while (DMA2D->CR & DMA2D_CR_START) 
  {
  }
}
/**
 * [DMA2D_CopyBufferWithAlpha 使用混合复制一个缓存区]
 * @param LayerIndex [显示层编号]
 * @param pSrc       [源地址]
 * @param pDst       [目地址]
 * @param xSize      [x方向大小]
 * @param ySize      [y方向大小]
 * @param OffLineSrc [源偏移值]
 * @param OffLineDst [目偏移值]
 */
static void DMA2D_CopyBufferWithAlpha(U32 LayerIndex, void * pSrc, void * pDst, U32 xSize, U32 ySize, U32 OffLineSrc, U32 OffLineDst)
{
  uint32_t PixelFormat;

  PixelFormat = GetPixelformat(LayerIndex);
  /*M2M 混合模式,最后一行传输完成中断模式*/
  DMA2D->CR      = 0x00000000UL | (1 << 9) | (0x2 << 16);   

  /* Set up pointers */
  DMA2D->FGMAR   = (U32)pSrc;                       
  DMA2D->OMAR    = (U32)pDst;                       
  DMA2D->FGOR    = OffLineSrc;                      
  DMA2D->OOR     = OffLineDst;

  DMA2D->BGMAR   = (U32)pDst; 
  DMA2D->BGOR    = OffLineDst; 

  /* Set up pixel format */  
  DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;  
  DMA2D->BGPFCCR = PixelFormat;
  DMA2D->OPFCCR = PixelFormat;

  /*  Set up size */
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize; 

  DMA2D->CR     |= DMA2D_CR_START;   

  /* Wait until transfer is done */
  while (DMA2D->CR & DMA2D_CR_START) 
  {
  }
}
/**
 * [DMA2D_FillBuffer 填充一个缓存区]
 * @param LayerIndex [填充层索引]
 * @param pDst       [目的地址]
 * @param xSize      [x方向大小]
 * @param ySize      [y方向大小]
 * @param OffLine    [行偏移值]
 * @param ColorIndex [填充的颜色]
 */
static void DMA2D_FillBuffer(U32 LayerIndex, void * pDst, U32 xSize, U32 ySize, U32 OffLine, U32 ColorIndex) 
{
  U32 PixelFormat;

  PixelFormat = GetPixelformat(LayerIndex);

  /*R2M模式*/
  DMA2D->CR      = 0x00030000UL | (1 << 9);        
  DMA2D->OCOLR   = ColorIndex;                     

  /* Set up pointers */
  DMA2D->OMAR    = (U32)pDst;                      

  /* Set up offsets */
  DMA2D->OOR     = OffLine;                        

  /* Set up pixel format */
  DMA2D->OPFCCR  = PixelFormat;                    

  /*  Set up size */
  DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;

  DMA2D->CR     |= DMA2D_CR_START; 

  /* Wait until transfer is done */
  while (DMA2D->CR & DMA2D_CR_START) 
  {
  }
}
/**
 * [GetBufferSize 返回显示区缓存的大小]
 * @param  LayerIndex [显示层索引]
 * @return            [缓存区大小以Byte为单位]
 */
static U32 GetBufferSize(U32 LayerIndex) 
{
  return (layer_prop[LayerIndex].xSize * layer_prop[LayerIndex].ySize * layer_prop[LayerIndex].BytesPerPixel);
}
/**
 * [LCD_LL_CopyBuffer LCD复制一个缓存区]
 * @param LayerIndex [显示层索引]
 * @param IndexSrc   [原地址]
 * @param IndexDst   [目地址]
 */
static void LCD_LL_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst) 
{
  U32 BufferSize, AddrSrc, AddrDst;

  BufferSize = GetBufferSize(LayerIndex);
  /*下面这个地址计算不是很明白。。。。。。。。。*/
  AddrSrc    = layer_prop[LayerIndex].address + BufferSize * IndexSrc;
  AddrDst    = layer_prop[LayerIndex].address + BufferSize * IndexDst;
  DMA2D_CopyBuffer(LayerIndex, (void *)AddrSrc, (void *)AddrDst, layer_prop[LayerIndex].xSize, layer_prop[LayerIndex].ySize, 0, 0);
  layer_prop[LayerIndex].buffer_index = IndexDst;
}

/**
  * @brief  Copy rectangle
  * @param  LayerIndex : Layer Index
  * @param  x0:          X0 position
  * @param  y0:          Y0 position
  * @param  x1:          X1 position
  * @param  y1:          Y1 position
  * @param  xSize:       X size. 
  * @param  ySize:       Y size.            
  * @retval None.
  */
static void LCD_LL_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize) 
{
  U32 BufferSize, AddrSrc, AddrDst;


  AddrSrc = layer_prop[LayerIndex].address + (y0 * layer_prop[LayerIndex].xSize + x0) * layer_prop[LayerIndex].BytesPerPixel;
  AddrDst = layer_prop[LayerIndex].address + (y1 * layer_prop[LayerIndex].xSize + x1) * layer_prop[LayerIndex].BytesPerPixel;
  DMA2D_CopyBuffer(LayerIndex, (void *)AddrSrc, (void *)AddrDst, xSize, ySize, layer_prop[LayerIndex].xSize - xSize, layer_prop[LayerIndex].xSize - xSize);
}

/**
  * @brief  Fill rectangle
  * @param  LayerIndex : Layer Index
  * @param  x0:          X0 position
  * @param  y0:          Y0 position
  * @param  x1:          X1 position
  * @param  y1:          Y1 position
  * @param  PixelIndex:  Pixel index.             
  * @retval None.
  */
static void LCD_LL_FillRect(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex) 
{
  U32 BufferSize, AddrDst;
  int xSize, ySize;

  if (GUI_GetDrawMode() == GUI_DM_XOR) 
  {   
    LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, NULL);
    LCD_FillRect(x0, y0, x1, y1);
    LCD_SetDevFunc(LayerIndex, LCD_DEVFUNC_FILLRECT, (void(*)(void))LCD_LL_FillRect);
  } 
  else 
  {
    xSize = x1 - x0 + 1;
    ySize = y1 - y0 + 1;
    BufferSize = GetBufferSize(LayerIndex);
    AddrDst = layer_prop[LayerIndex].address + BufferSize * layer_prop[LayerIndex].buffer_index + (y0 * layer_prop[LayerIndex].xSize + x0) * layer_prop[LayerIndex].BytesPerPixel;
    DMA2D_FillBuffer(LayerIndex, (void *)AddrDst, xSize, ySize, layer_prop[LayerIndex].xSize - xSize, PixelIndex);
  } 
}

/**
  * @brief  Draw indirect color bitmap
  * @param  pSrc: pointer to the source
  * @param  pDst: pointer to the destination
  * @param  OffSrc: offset source
  * @param  OffDst: offset destination
  * @param  PixelFormatDst: pixel format for destination
  * @param  xSize: X size
  * @param  ySize: Y size
  * @retval None
  */
static void DMA2D_DrawBitmapL8(void * pSrc, void * pDst,  U32 OffSrc, U32 OffDst, U32 PixelFormatDst, U32 xSize, U32 ySize)
{ 
  /* Set up mode */
  DMA2D->CR      = 0x00010000UL | (1 << 9);         /* Control Register (Memory to memory with pixel format conversion and TCIE) */

  /* Set up pointers */
  DMA2D->FGMAR   = (U32)pSrc;                       /* Foreground Memory Address Register (Source address) */
  DMA2D->OMAR    = (U32)pDst;                       /* Output Memory Address Register (Destination address) */

  /* Set up offsets */
  DMA2D->FGOR    = OffSrc;                          /* Foreground Offset Register (Source line offset) */
  DMA2D->OOR     = OffDst;                          /* Output Offset Register (Destination line offset) */

  /* Set up pixel format */
  DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_L8;             /* Foreground PFC Control Register (Defines the input pixel format) */
  DMA2D->OPFCCR  = PixelFormatDst;                   /* Output PFC Control Register (Defines the output pixel format) */

  /* Set up size */
  DMA2D->NLR     = (U32)(xSize << 16) | ySize;       /* Number of Line Register (Size configuration of area to be transfered) */

  /* Execute operation */
  DMA2D->CR     |= DMA2D_CR_START;                   /* Start operation */

  /* Wait until transfer is done */
  while (DMA2D->CR & DMA2D_CR_START)
  {
  } 
}

/**
  * @brief  Draw 16bpp bitmap file
  * @param  LayerIndex: Layer Index
  * @param  x:          X position
  * @param  y:          Y position
  * @param  p:          pointer to destination address
  * @param  xSize:      X size
  * @param  ySize:      Y size
  * @param  BytesPerLine
  * @retval None
  */
static void LCD_LL_DrawBitmap16bpp(int LayerIndex, int x, int y, U16 const * p, int xSize, int ySize, int BytesPerLine)
{
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;

  BufferSize = GetBufferSize(LayerIndex);
  AddrDst = layer_prop[LayerIndex].address + BufferSize * layer_prop[LayerIndex].buffer_index + (y * layer_prop[LayerIndex].xSize + x) * layer_prop[LayerIndex].BytesPerPixel;
  OffLineSrc = (BytesPerLine / 2) - xSize;
  OffLineDst = layer_prop[LayerIndex].xSize - xSize;
  DMA2D_CopyBuffer(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

static void LCD_LL_DrawBitmap32bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine)
{
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;

  BufferSize = GetBufferSize(LayerIndex);
  AddrDst = layer_prop[LayerIndex].address + BufferSize * layer_prop[LayerIndex].buffer_index + (y * layer_prop[LayerIndex].xSize + x) * layer_prop[LayerIndex].BytesPerPixel;
  OffLineSrc = (BytesPerLine / 4) - xSize;
  OffLineDst = layer_prop[LayerIndex].xSize - xSize;
  DMA2D_CopyBufferWithAlpha(LayerIndex, (void *)p, (void *)AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

/**
  * @brief  Draw 8bpp bitmap file
  * @param  LayerIndex: Layer Index
  * @param  x:          X position
  * @param  y:          Y position
  * @param  p:          pointer to destination address 
  * @param  xSize:      X size
  * @param  ySize:      Y size
  * @param  BytesPerLine
  * @retval None
  */
static void LCD_LL_DrawBitmap8bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine)
{
  U32 BufferSize, AddrDst;
  int OffLineSrc, OffLineDst;
  U32 PixelFormat;

  BufferSize = GetBufferSize(LayerIndex);
  AddrDst = layer_prop[LayerIndex].address + BufferSize * layer_prop[LayerIndex].buffer_index + (y * layer_prop[LayerIndex].xSize + x) * layer_prop[LayerIndex].BytesPerPixel;
  OffLineSrc = BytesPerLine - xSize;
  OffLineDst = layer_prop[LayerIndex].xSize - xSize;
  PixelFormat = GetPixelformat(LayerIndex);
  DMA2D_DrawBitmapL8((void *)p, (void *)AddrDst, OffLineSrc, OffLineDst, PixelFormat, xSize, ySize);
}

/**
  * @brief  Line Event callback.
  * @param  hltdc_eval: pointer to a LTDC_HandleTypeDef structure that contains
  *                the configuration information for the specified LTDC.
  * @retval None
  */
void HAL_LTDC_LineEvenCallback(LTDC_HandleTypeDef *hltdc_eval) {
  
  U32 Addr;
  U32 layer;

  for (layer = 0; layer < GUI_NUM_LAYERS; layer++)
  {
    if (layer_prop[layer].pending_buffer >= 0) 
    {
      /* Calculate address of buffer to be used  as visible frame buffer */
      Addr = layer_prop[layer].address + \
             layer_prop[layer].xSize * layer_prop[layer].ySize * layer_prop[layer].pending_buffer * layer_prop[layer].BytesPerPixel;
      
      __HAL_LTDC_LAYER(hltdc_eval, layer)->CFBAR = Addr;
     
      __HAL_LTDC_RELOAD_CONFIG(hltdc_eval);
      
      /* Notify STemWin that buffer is used */
      GUI_MULTIBUF_ConfirmEx(layer, layer_prop[layer].pending_buffer);

      /* Clear pending buffer flag of layer */
      layer_prop[layer].pending_buffer = -1;
    }
  }
 
  HAL_LTDC_ProgramLineEvent(hltdc_eval, 0);

}
/**
  * @brief  Tearing Effect DSI callback.
  * @param  hdsi: pointer to a DSI_HandleTypeDef structure that contains
  *               the configuration information for the DSI.
  * @retval None
  */
void HAL_DSI_TearingEffectCallback(DSI_HandleTypeDef *hdsi)
{
  uint32_t index = 0;

  if(!LCD_Refershing)
  {
    for(index = 0; index < GUI_NUM_LAYERS; index ++)
    {
      if(layer_prop[index].pending_buffer >= 0)
      {
        GUI_MULTIBUF_ConfirmEx(index,  layer_prop[index].pending_buffer);
        layer_prop[index].pending_buffer = -1;
      } 
    }    
    LCD_Refershing = 1;
    LCD_ActiveRegion = 1;
    HAL_DSI_Refresh(hdsi); 
  }
}  

/**
  * @brief  End of Refresh DSI callback.
  * @param  hdsi: pointer to a DSI_HandleTypeDef structure that contains
  *               the configuration information for the DSI.
  * @retval None
  */
//void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi)
//{
//  uint32_t index = 0;
//
//  if(LCD_ActiveRegion < ZONES )
//  {
//    LCD_Refershing = 1;
//    /* Disable DSI Wrapper */
//    __HAL_DSI_WRAPPER_DISABLE(hdsi);
//    for(index = 0; index < GUI_NUM_LAYERS; index ++)
//    {
//      /* Update LTDC configuaration */
//      LTDC_LAYER(&LTDC_Handle, index)->CFBAR  = LCD_Addr[index] + LCD_ActiveRegion  * HACT * 2;
//    }
//    __HAL_LTDC_RELOAD_CONFIG(&LTDC_Handle);
//    __HAL_DSI_WRAPPER_ENABLE(hdsi);
//    LCD_SetUpdateRegion(LCD_ActiveRegion++);
//    /* Refresh the right part of the display */
//    HAL_DSI_Refresh(hdsi);
//  }
//  else
//  {
//    LCD_Refershing = 0;
//    
//    __HAL_DSI_WRAPPER_DISABLE(&DSI_Handle);
//    for(index = 0; index < GUI_NUM_LAYERS; index ++)
//    {
//      LTDC_LAYER(&LTDC_Handle, index)->CFBAR  = LCD_Addr[index];
//    }
//    __HAL_LTDC_RELOAD_CONFIG(&LTDC_Handle);
//    __HAL_DSI_WRAPPER_ENABLE(&DSI_Handle);  
//    LCD_SetUpdateRegion(0); 
//  }
//}

/*************************** End of file ****************************/
