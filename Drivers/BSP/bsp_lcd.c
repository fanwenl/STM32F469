/*
*************************************************************************************************
*文件：bsp_lcd.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-06-01
*描述：STM32F469NI-Discovery开发套件4"TFT液晶驱动程序。
**************************************************************************************************
*/

#include "bsp_lcd.h"
#include "./Font/font24.c"
#include "./Font/font20.c"
#include "./Font/font16.c"
#include "./Font/font12.c"
#include "./Font/font8.c"
#include "./Font/fonts.h"

DMA2D_HandleTypeDef hdma2d_eval;
static DSI_HandleTypeDef	DSI_Handle;
static LTDC_HandleTypeDef LTDC_Handle;
static DSI_VidCfgTypeDef  DSI_VideoStru;

static LCD_DrawPropTypeDef DrawProp[LTDC_MAX_LAYER_NUMBER];
static void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);
static void LL_ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode);

static uint32_t  ActiveLayer = LTDC_ACTIVE_LAYER_BACKGROUND;
static uint32_t lcd_x_size = OTM8009A_800X480_WIDTH;
static uint32_t lcd_y_size = OTM8009A_800X480_HEIGHT;

#define ABS(X)                 ((X) > 0 ? (X) : -(X))



uint8_t LCD_Init(LCD_OrientationTypeDef orientation)
{

	DSI_PLLInitTypeDef	DSI_PLLInit;
	static RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
	
	uint32_t LcdClk = 27429;
	uint32_t LaneByteClk_kHz = 62500;

	uint32_t VSA;  			/*垂直同步段*/
	uint32_t VFP; 				/*垂直前沿*/
	uint32_t VBP;				/*垂直后沿*/
	uint32_t VACT;				/*垂直有效像素*/
	
	uint32_t HSA;				/*水平同步段*/
	uint32_t HFP;				/*水平前沿*/
	uint32_t HBP;				/*水平后沿*/
	uint32_t HACT;				/*水平有效像素*/

	VSA = OTM8009A_480X800_VSYNC;  		/*1*/
	VFP =	OTM8009A_480X800_VFP;			/*16*/
	VBP = OTM8009A_480X800_VBP;			/*16*/
	
	HSA = OTM8009A_480X800_HSYNC;			/*2*/
	HFP = OTM8009A_480X800_HFP;			/*20*/
	HBP = OTM8009A_480X800_HBP;			/*20*/

	if(orientation == LCD_ORIENTATION_LANDSCAPE)/*横屏显示模式*/
	{
		lcd_y_size = OTM8009A_800X480_HEIGHT;
		lcd_x_size = OTM8009A_800X480_WIDTH;
	}
	else
	{
		lcd_x_size = OTM8009A_480X800_WIDTH;
		lcd_y_size = OTM8009A_480X800_HEIGHT;
	}

	VACT = lcd_y_size;
	HACT = lcd_x_size;
		/*复位LCD*/
	LCD_Reset();
	/*初始化DSI相关外设*/
	LCD_MspInit();
/********************************DSI配置*********************************************/
	DSI_Handle.Instance = DSI;

	HAL_DSI_DeInit(&DSI_Handle);

//	DSI_Handle.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE; 
	/*TxEscapeCkdiv = (laneByClk_kHz (500MHz/8) /TxEsc),TxEsc最大20M这里4分频*/
	DSI_Handle.Init.TXEscapeCkdiv = 4;
	DSI_Handle.Init.NumberOfLanes = DSI_TWO_DATA_LANES;

	DSI_PLLInit.PLLNDIV = 125;
	DSI_PLLInit.PLLIDF  = DSI_PLL_IN_DIV2;
	DSI_PLLInit.PLLODF  = DSI_PLL_OUT_DIV1;

	HAL_DSI_Init(&DSI_Handle, &DSI_PLLInit);
	
	DSI_VideoStru.VirtualChannelID = LCD_OTM8009A_ID; /*定义LTDC和DSI接口的通道*/
	DSI_VideoStru.ColorCoding      = DSI_RGB888; /*定义DSI接口颜色模式*/
	DSI_VideoStru.LooselyPacked    = DSI_LOOSELY_PACKED_DISABLE; /*禁止Loosely*/
	DSI_VideoStru.Mode             = DSI_VID_MODE_BURST;/*视屏突发模式*/
	DSI_VideoStru.PacketSize       = HACT; /*水平像素有效值*/
	/*如果配置为突发模式，下面这两个配置无效*/
	DSI_VideoStru.NumberOfChunks   = 0;		/*the video line is transmitted in a single packet*/
	DSI_VideoStru.NullPacketSize   = 0xFFF;/*使能填充无效的数据在每一行，无效的个数为0xFF*/
	
	/*HSYNC有效像素极性，和实际LCD芯片有关*/
	DSI_VideoStru.HSPolarity       = DSI_HSYNC_ACTIVE_HIGH; 
	DSI_VideoStru.VSPolarity       = DSI_VSYNC_ACTIVE_HIGH; 
	DSI_VideoStru.DEPolarity       = DSI_DATA_ENABLE_ACTIVE_HIGH;

	/*配置行扫描时序，以时钟周期表示 用户手册page591*/
	DSI_VideoStru.HorizontalSyncActive = HSA * (LaneByteClk_kHz / LcdClk);  /*行扫描起始有效信号*/
	DSI_VideoStru.HorizontalBackPorch  = HBP *(LaneByteClk_kHz / LcdClk); /*行后沿*/
	DSI_VideoStru.HorizontalLine       = (HACT + HSA + HBP + HFP) *(LaneByteClk_kHz / LcdClk);/*扫描一行所需要的时间*/
	
	/*配置场扫描，以行为单位表示*/
	DSI_VideoStru.VerticalSyncActive = VSA;
	DSI_VideoStru.VerticalBackPorch  = VBP; 
	DSI_VideoStru.VerticalFrontPorch = VFP;
	DSI_VideoStru.VerticalActive     = VACT;  

	/*使能低功耗发送命令*/
	DSI_VideoStru.LPCommandEnable = DSI_LP_COMMAND_ENABLE;   

	/*这部分不会计算，用户手册page 562*/
	DSI_VideoStru.LPLargestPacketSize     = 64;
	DSI_VideoStru.LPVACTLargestPacketSize = 64;

	/*使能在时序部分发送低功耗命令*/
	DSI_VideoStru.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE; /*允许发送低功耗命令在HFP周期*/
	DSI_VideoStru.LPHorizontalBackPorchEnable  = DSI_LP_HBP_ENABLE;
	DSI_VideoStru.LPVerticalActiveEnable       = DSI_LP_VACT_ENABLE;
	DSI_VideoStru.LPVerticalFrontPorchEnable   = DSI_LP_VFP_ENABLE;
	DSI_VideoStru.LPVerticalBackPorchEnable    = DSI_LP_VBP_ENABLE;/* Allow sending LP commands during VBP period */
	DSI_VideoStru.LPVerticalSyncActiveEnable   = DSI_LP_VSYNC_ENABLE;/* Allow sending LP commands during VSync = VSA period */

	/*禁止一帧传输完成的应答信号*/
//	DSI_VideoStru.FrameBTAAcknowledgeEnable    = DSI_FBTAA_DISABLE;


	/*配置DSI的 Video模式*/
	 HAL_DSI_ConfigVideoMode(&DSI_Handle, &DSI_VideoStru);

	/*启动DSI*/
	 HAL_DSI_Start(&DSI_Handle);

/*******************************LTDC配置*********************************************/
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN       = 384;
	PeriphClkInitStruct.PLLSAI.PLLSAIR       = 7;
	PeriphClkInitStruct.PLLSAIDivR           = RCC_PLLSAIDIVR_2;
	
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/*配置LTDC时序*/
	LTDC_Handle.Instance                = LTDC;    /*设置LTDC的寄存器地址*/
	
	LTDC_Handle.Init.PCPolarity         = LTDC_PCPOLARITY_IPC;
	
	LTDC_Handle.Init.HorizontalSync     = (HSA - 1);
	LTDC_Handle.Init.AccumulatedHBP     = (HSA + HBP - 1);
	
	LTDC_Handle.Init.AccumulatedActiveW = (lcd_x_size + HSA +HBP - 1);
	LTDC_Handle.Init.TotalWidth         = (lcd_x_size + HSA +HBP + HFP - 1);

	/*配置默认背景颜色*/
	LTDC_Handle.Init.Backcolor.Blue  = 0x00 ;
	LTDC_Handle.Init.Backcolor.Red   = 0x00 ;
	LTDC_Handle.Init.Backcolor.Green = 0x00 ;

	HAL_LTDC_StructInitFromVideoConfig(&LTDC_Handle, &DSI_VideoStru);

	HAL_LTDC_Init(&LTDC_Handle);

	LCD_SetFont(&LCD_DEFAULT_FONT);
	
/********************************OTM8009A配置*********************************************/
	  
	OTM8009A_Init(DSI_VideoStru.ColorCoding, orientation);

	return LCD_OK;
  
}
/*
**************************************************************************************************
*描述：LCD复位函数
*参数：无
*返回：无
*note：该函数是硬件复位，复位引脚链接在PH7
* ************************************************************************************************
 */
void LCD_Reset(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*使能GPIOH的时钟*/
	__HAL_RCC_GPIOH_CLK_ENABLE();
	/*配置PH7的引脚为开漏输出*/
	GPIO_InitStructure.Pin   = GPIO_PIN_7;
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStructure.Pull  = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	
	HAL_GPIO_Init(GPIOH,&GPIO_InitStructure);
	/*复位(低有效)*/
	HAL_GPIO_WritePin(GPIOH,GPIO_PIN_7,GPIO_PIN_RESET);

	HAL_Delay(20);/*延时20ms*/
	/*恢复*/
	HAL_GPIO_WritePin(GPIOH,GPIO_PIN_7,GPIO_PIN_SET);
	
	HAL_Delay(20);	
}
/*
**************************************************************************************************
*描述：DSI相关外设初始函数
*参数：无
*返回：无
*note：使能LTDC、DMA2D、DSI时钟，复位相关外设，配置DSI中断
* ************************************************************************************************
 */
void LCD_MspInit(void)
{
	/*使能LTDC时钟*/
   __HAL_RCC_LTDC_CLK_ENABLE();
	/*复位LTDC IP*/
	__HAL_RCC_LTDC_FORCE_RESET();
  	__HAL_RCC_LTDC_RELEASE_RESET();

	/*使能DMA2D时钟*/
   __HAL_RCC_DMA2D_CLK_ENABLE();
	/*复位DMA2D IP*/
	__HAL_RCC_DMA2D_FORCE_RESET();
  	__HAL_RCC_DMA2D_RELEASE_RESET();

	/*使能DSI时钟*/
   __HAL_RCC_DSI_CLK_ENABLE();
	/*复位DSI IP*/
	__HAL_RCC_DSI_FORCE_RESET();
  	__HAL_RCC_DSI_RELEASE_RESET();
	
	/*配置LTDC中断并使能该中断*/
	HAL_NVIC_SetPriority(LTDC_IRQn,3,0);
	HAL_NVIC_EnableIRQ(LTDC_IRQn);

	/*配置DSI中断并使能该中断*/
	HAL_NVIC_SetPriority(DSI_IRQn,3,0);	/*这部分中断在哪里在哪里调用*/
	HAL_NVIC_EnableIRQ(DSI_IRQn);
}
/**
 * [LCD_GetXSize 获取LCD的宽(以像素表示)]
 * @return  [LCD的宽度]
 */
uint32_t LCD_GetXSize(void)
{
	return (lcd_x_size);
}
/**
 * [LCD_GetYSize 获取LCD的显示高度(以行为单位)]
 * @return  [LCD的高度]
 */
uint32_t LCD_GetYSize(void)
{
	return(lcd_y_size);
}
/**
 * [LCD_SetXSize 设置LCD显示范围的宽]
 * @param imageWidthPixels [图像的宽度单位是像素]
 */
void LCD_SetXSize(uint16_t imageWidthPixels)
{
	LTDC_Handle.LayerCfg[ActiveLayer].ImageWidth = imageWidthPixels;
}

/**
 * [LCD_SetYSize 设置LCD显示范围的高]
 * @param imageHeighPixels [图像显示的高度单位行]
 */
void LCD_SetYSize(uint16_t imageHeighPixels)
{
	LTDC_Handle.LayerCfg[ActiveLayer].ImageHeight = imageHeighPixels;
}
/**
 * LCD默图层初始化函数
 * @param LayerIndex [图层序号，eg：1 or 2]
 * @param FB_Address [颜色缓存区地址]
 */

void LCD_LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address)
{
	LCD_LayerCfgTypeDef LayerCfgStru;

	LayerCfgStru.WindowX0        = 0;
	LayerCfgStru.WindowX1        = LCD_GetXSize();
	LayerCfgStru.WindowY0        = 0;
	LayerCfgStru.WindowY1        = LCD_GetYSize(); 
	LayerCfgStru.PixelFormat     = LTDC_PIXEL_FORMAT_ARGB8888;
	LayerCfgStru.Alpha           = 255;
	LayerCfgStru.Alpha0          = 0;
	LayerCfgStru.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA; 
	LayerCfgStru.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA; 
	LayerCfgStru.ImageWidth      = LCD_GetXSize();
	LayerCfgStru.ImageHeight     = LCD_GetYSize(); 
	LayerCfgStru.Backcolor.Blue  = 0;
	LayerCfgStru.Backcolor.Green = 0;
	LayerCfgStru.Backcolor.Red   = 0;
	LayerCfgStru.FBStartAdress   = FB_Address;

	HAL_LTDC_ConfigLayer(&LTDC_Handle, &LayerCfgStru, LayerIndex);

	DrawProp[LayerIndex].BackColor = LCD_COLOR_RED;
	DrawProp[LayerIndex].TextColor = LCD_COLOR_BLUE;
	DrawProp[LayerIndex].pFont     = &Font24;
}
/**
 * [LCD_SelectLayer 选择要显示的层]
 * @param LayerIndex [要显示的层的标号]
 */
void LCD_SelectLayer(uint32_t LayerIndex)
{
	ActiveLayer = LayerIndex;
}
/**
 * [DSI_IO_WriteCmd DSI写命令函数]
 * @param NbParams [命令个数]
 * @param *pParams [命令参数指针]
 */
void DSI_IO_WriteCmd(uint32_t NbParams, uint8_t *pParams)
{
	if (NbParams <= 1)
	{
		HAL_DSI_ShortWrite(&DSI_Handle, 
						   DSI_VideoStru.VirtualChannelID,
						   DSI_DCS_SHORT_PKT_WRITE_P1, 
						   pParams[0],
						   pParams[1]);
	}
	else
	{
		HAL_DSI_LongWrite(&DSI_Handle, 
						  DSI_VideoStru.VirtualChannelID,
						  DSI_DCS_LONG_PKT_WRITE,
						  NbParams,
						  pParams[NbParams],
						  pParams);
	}
}
/**
 * [LCD_DisplayOn LCD打开函数]
 */
void LCD_DisplayOn(void)
{
	HAL_DSI_ShortWrite( &DSI_Handle,
						DSI_VideoStru.VirtualChannelID,
						DSI_DCS_SHORT_PKT_WRITE_P1,
						OTM8009A_CMD_DISPON,
						0x00);
}

/**
 * [LCD_DisplayOn LCD关闭函数]
 */
void LCD_DisplayOff(void)
{
	HAL_DSI_ShortWrite( &DSI_Handle,
						DSI_VideoStru.VirtualChannelID,
						DSI_DCS_SHORT_PKT_WRITE_P1,
						OTM8009A_CMD_DISPOFF,
						0x00);
}
/**
 * [LCD_SetFont 设置LCD的字体]
 * @param fonts [字体指针]
 */
void LCD_SetFont(sFONT *fonts)
{
	DrawProp[ActiveLayer].pFont = fonts;
}
/**
 * [LCD_GetFont 获取LCD设置的字体]
 * @return  [返回使用的当前层的字体]
 */
sFONT *LCD_GetFont(void)
{
	return(DrawProp[ActiveLayer].pFont);
}
/**
 * [LCD_DrawPixel LCD写像素]
 * @param Xpos     [x点的坐标]
 * @param Ypos     [y点的坐标]
 * @param RGBValue [像素点的值ARGB模式(8888模式)]
 */
void LCD_DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGBValue)
{
	/*向FBStartAdress的颜色缓存地址写颜色的值，通过(Ypos * LCD_GetXSize() + Xpos)表达式先计算出需要写的像素点的偏移位置*/
	/*然后在乘以4变成字节数，+FBStartAdres就是该像素的绝对地址，由于是一个地址需要将这个强制转换为一个指针，最后对这个指针复制*/

	*(uint32_t *)(LTDC_Handle.LayerCfg[ActiveLayer].FBStartAdress + (4 * (Ypos * LCD_GetXSize() + Xpos))) = RGBValue;
}
/**
 * [LCD_DisplayChar LCD显示一个字符]
 * @param Xpos [X点的坐标]
 * @param Ypos [Y点的坐标]
 * @param Ascii [需要显示的字符]
 */
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  uint32_t i = 0, j = 0;
  uint16_t height, width;
  uint8_t  offset;
  uint8_t  *pchar;
  const uint8_t *pc;
  uint32_t line;

  height = DrawProp[ActiveLayer].pFont->Height;
  width  = DrawProp[ActiveLayer].pFont->Width;

  /*c指向要显示的要显示的字符的字摸数据，应为Ascii第一个字符是空格，所以要(Ascii-' ')*/
  /*(Ascii-' ') * height * ((width + 7) / 8 计算要显示的字符在字体table中的绝对位置*/
  pc = &DrawProp[ActiveLayer].pFont->table[(Ascii-' ') * height * ((width + 7) / 8)];

  /*offset计算有几个像素点是没有取摸的，((width + 7)/8)这个是计算一行取摸需要几个字节*/
  offset =  8 *((width + 7)/8) -  width ;  

  for(i = 0; i < height; i++)     /*i是字摸的行数*/
  {
  	/*C是传进来的一个字符字摸起始的数据，pchar是控制字摸的第几行的，也就是行偏移地址*/
    pchar = ((uint8_t *)pc + (width + 7)/8 * i);

    switch(((width + 7)/8))
    {

    case 1:
    
      line =  pchar[0];                         			/*每行只有一个数据*/
      break;

    case 2:
      line =  (pchar[0]<< 8) | pchar[1];					/*每行有两个数据，进行拼接*/
      break;

    case 3:
    default:
      line =  (pchar[0]<< 16) | (pchar[1]<< 8) | pchar[2]; /*每行有三个数据*/
      break;
    }

    for (j = 0; j < width; j++)								/*字摸列像素点填充控制*/
    {
      if(line & (1 << (width- j + offset- 1)))/*每一个像素点控制，(width- j + offset- 1)的偏移值*/
      {
        LCD_DrawPixel((Xpos + j), Ypos, DrawProp[ActiveLayer].TextColor);
      }
      else
      {
        LCD_DrawPixel((Xpos + j), Ypos, DrawProp[ActiveLayer].BackColor);
      }
    }
    Ypos++;
  }
}
/**
 * [LCD_DsiplayStringAt 显示一个字符串]
 * @param Xpos [x点的坐标（像素）]
 * @param Ypos [y点的坐标（像素）]
 * @param Text [指向显示的字符串]
 * @param Mode [显示模式]
 *        @arg CENTER_MODE
 *        @arg RIGHT_MODE
 *        @arg LEFT_MODE
 */
void LCD_DsiplayStringAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Text_AlignModeTypdef Mode)
{
	uint8_t *ptr =Text;
	uint16_t size = 0, xsize = 0;
	uint16_t refcolumn = 1, i = 0;

	while(*ptr++) size++;    /*计算显示字符的个数*/

	xsize = (LCD_GetXSize()/DrawProp[ActiveLayer].pFont->Width); /*计算液晶用这种字体最多能显示多少个字符*/
 
 	/*显示模式，主要计算行起始的显示地址，refcolumn x方向坐标*/
	switch(Mode)
	{
		case CENTER_MODE:

			refcolumn = Xpos + ((xsize - size) * DrawProp[ActiveLayer].pFont->Width)/2;
			break;

		case LEFT_MODE:

			refcolumn = Xpos;
			break;

		case RIGHT_MODE:

			refcolumn = - Xpos + ((xsize - size)*DrawProp[ActiveLayer].pFont->Width);
		default:

			refcolumn = Xpos;
			break;
	}
	/*判断是否超出显示范围*/
	if ((refcolumn < 1) || (refcolumn >= LCD_GetXSize()))
  {
    refcolumn = 1;
  }
	/*显示字符，后面这个常常的判断--判断剩余的像素点能不能显示一个字符*/
	while((*Text != 0) & (((LCD_GetXSize() - (i*DrawProp[ActiveLayer].pFont->Width)) & 0xFFFF) >= DrawProp[ActiveLayer].pFont->Width))
	{
		LCD_DisplayChar(refcolumn, Ypos, *Text);

		refcolumn += DrawProp[ActiveLayer].pFont->Width;
		/*地址累加 下一个要显示的字符*/
		Text++;
		i++;
	}
}
/**
 * [LCD_DisplayStringAtLine x坐标从0开始显示一行字符串]
 * @param Line [要显示字符串的行数，通过LINE的宏定义可以转换为像素]
 *        @arg 整数的数字，根据字体的高度和液晶的方式可以计算出最大的行数
 * @param pstr [指向要显示的字符串]
 */
void LCD_DisplayStringAtLine(uint16_t Line, uint8_t *pstr)
{
	LCD_DsiplayStringAt(0, LINE(Line), pstr, LEFT_MODE);
}
/**
 * [LCD_DrawLine LCD画线命令(在当前激活的层，两点划线)]
 * @param x1 [第一个点的x坐标]
 * @param y1 [第二个点的y坐标]
 * @param x2 [第一个点的x坐标]
 * @param y2 [第二个点的y坐标]
 */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
 int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
  curpixel = 0;

  deltax = ABS(x2 - x1);        /*计算X方向的距离*/
  deltay = ABS(y2 - y1);        /*计算Y方向的距离*/
  x = x1;                       /*设置起始点的x坐标*/
  y = y1;                       /*设置起始点的y坐标*/

  if (x2 >= x1)                 /* 判断x方向的增长方式*/
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          
  {
    xinc1 = -1;
    xinc2 = -1;
  }

  if (y2 >= y1)                 /* 判断y方向的的增长方式*/
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                         
  {
    yinc1 = -1;
    yinc2 = -1;
  }

  if (deltax >= deltay)         /*设置参考轴，以及根据参考轴的增长数值，中心点划线算法*/
  {									 /*点(xinc1,yinc1)控制右上角的点，点(xinc2,yinc2)控制右边的点*/
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /*以X轴的像素点为总的像素点*/
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax; 
    numpixels = deltay;         /*以Y轴的像素点为总的像素点*/
  }

  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    LCD_DrawPixel(x, y, DrawProp[ActiveLayer].TextColor);
    num += numadd;                            /* Increase the numerator by the top of the fraction */
    if (num >= 0)                           /*近似点为右上角的点*/
    {
      num -= den;                             /* Calculate the new numerator value */
      x += xinc1;                             /* Change the x as appropriate */
      y += yinc1;                             /* Change the y as appropriate */
    }
    x += xinc2;                               /* Change the x as appropriate */
    y += yinc2;                               /* Change the y as appropriate */
  }
}
/**
 * [LCD_DrawHLine 画一条水平的线]
 * @param Xpos   [起点x坐标]
 * @param Ypos   [起点y坐标]
 * @param Length [线的长度(像素表示)]
 */
void LCD_DrawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
	uint32_t Xaddr = 0;

	/*¼ÆËãÏßµÄÆðÊ¼µØÖ·£¬Ò²¾ÍÊÇÆðÊ¼ÏñËØµãµÄµØÖ·*/
	Xaddr = (LTDC_Handle.LayerCfg[ActiveLayer].FBStartAdress) + 4 * (Ypos * LCD_GetXSize() + Xpos);
	
	/*»­Ïßº¯Êý£¬¼¤»îµÄ²ã£¬ÆðÊ¼µØÖ·£¬ÏßµÄ³¤¶È£¬ÏßµÄ¿í¶È£¨Ò²¾ÍÊÇyÖáµÄsize£©¡¢ÏßµÄÑÕÉ«*/
	LL_FillBuffer(ActiveLayer, (uint32_t *)Xaddr, Length, 1, 0, DrawProp[ActiveLayer].TextColor);
}
/**
 * [LCD_DrawHLine 画一条垂直的线]
 * @param Xpos   [起点x坐标]
 * @param Ypos   [起点y坐标]
 * @param Length [线的长度(像素表示)]
 */
void LCD_DrawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
	uint32_t Xaddr = 0;

	/*¼ÆËãÏßµÄÆðÊ¼µØÖ·£¬Ò²¾ÍÊÇÆðÊ¼ÏñËØµãµÄµØÖ·*/
	Xaddr = (LTDC_Handle.LayerCfg[ActiveLayer].FBStartAdress) + 4 * (Ypos * LCD_GetXSize() + Xpos);
	
	/*»­Ïßº¯Êý£¬¼¤»îµÄ²ã£¬ÆðÊ¼µØÖ·£¬ÏßµÄ³¤¶È£¨Ò²¾ÍÊÇxÖáµÄsize£©£¬ÏßµÄ¿í¶È£¨Ò²¾ÍÊÇyÖáµÄsize£©¡¢ÏßµÄÑÕÉ«*/
	LL_FillBuffer(ActiveLayer, (uint32_t *)Xaddr, 1, Length, LCD_GetXSize() - 1, DrawProp[ActiveLayer].TextColor);
}
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
	/*Ë®Æ½µÄÁ½ÌõÏß*/
	LCD_DrawHLine(Xpos, Ypos, Width);
	LCD_DrawHLine(Xpos, (Ypos + Height), Width);

	/*´¹Ö±µÄÁ½ÌõÏß*/
	LCD_DrawVLine(Xpos, Ypos, Height);
	LCD_DrawVLine((Xpos + Width), Ypos, Height);
}
/**
 * [LCD_DrawCircle 画圆函数]
 * @param Xpos   [圆心坐标]
 * @param Ypos   [圆心坐标]
 * @param Radius [圆的半径]
 */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
	uint32_t CurX;
	uint32_t CurY;
	int32_t p;

	CurX = 0;
	CurY = Radius;
	p = 5 - 4*Radius; /*p = 5/4 - r 扩大四倍*/

	/*根据圆的对称性将，圆分成8部分，从(0,r)点在第一象限开始绘制*/
	while(CurX <= CurY)
	{
		LCD_DrawPixel((Xpos + CurX), (Ypos + CurY), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos - CurX), (Ypos + CurY), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos + CurX), (Ypos - CurY), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos - CurX), (Ypos - CurY), DrawProp[ActiveLayer].TextColor);

		LCD_DrawPixel((Xpos + CurY), (Ypos + CurX), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos - CurY), (Ypos + CurX), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos + CurY), (Ypos - CurX), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos - CurY), (Ypos - CurX), DrawProp[ActiveLayer].TextColor);

		if (p < 0)         /*中点画圆法，增量p*/
		{
			p += 8 * CurX + 12;
		}
		else 
		{
			p += 8 * (CurX - CurY) + 20;
			CurY--;
		}

		CurX++;
	}
}
/**
 * [BSP_LCD_DrawEllipse 画一个椭圆]
 * @param Xpos    [椭圆中心点X坐标]
 * @param Ypos    [椭圆中心点Y坐标]
 * @param XRadius [椭圆X轴半径]
 * @param YRadius [椭圆Y轴半径]
 */
void LCD_DrawEllipse(uint16_t Xpos, uint16_t Ypos, uint16_t XRadius, uint16_t YRadius)
{
  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
  float K = 0, rad1 = 0, rad2 = 0;

  rad1 = XRadius;
  rad2 = YRadius;

  K = (float)(rad2/rad1);

  do {
	    LCD_DrawPixel((Xpos-(uint16_t)(x/K)), (Ypos+y), DrawProp[ActiveLayer].TextColor);
	    LCD_DrawPixel((Xpos+(uint16_t)(x/K)), (Ypos+y), DrawProp[ActiveLayer].TextColor);
	    LCD_DrawPixel((Xpos+(uint16_t)(x/K)), (Ypos-y), DrawProp[ActiveLayer].TextColor);
	    LCD_DrawPixel((Xpos-(uint16_t)(x/K)), (Ypos-y), DrawProp[ActiveLayer].TextColor);

    e2 = err;
    if (e2 <= x) {
      err += ++x*2+1;
      if (-y == x && e2 <= y) e2 = 0;
    }
    if (e2 > y) err += ++y*2+1;
  }
  while (y <= 0);
}
//void LCD_DrawPolygon(pPoint Points, uint16_t PointCount)
//{
//
//}
/*使用<<计算机图形学>>中的画椭圆的方式
void LCD_DrawEllipse(uint16_t Xpos, uint16_t Ypos, uint16_t XRadius, uint16_t YRadius)
{
	uint32_t XR2 = XRadius * XRadius;
	uint32_t YR2 = YRadius * YRadius;
	uint32_t twoXR2 = 2 * XR2;
	uint32_t twoYR2 = 2 * YR2;
	uint16_t x = 0;
	uint16_t y = YRadius;
	uint32_t px = 0;
	uint32_t py = twoXR2 * y;
	int32_t p = (uint32_t)((YR2 - (XR2 * YRadius) + (0.25 * XR2)) + 0.5);
	do
	{
		LCD_DrawPixel((Xpos + x), (Ypos + y), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos - x), (Ypos + y), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos + x), (Ypos - y), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos - x), (Ypos - y), DrawProp[ActiveLayer].TextColor);
		x++;
		px += twoYR2;
		if (p < 0)
		{
			p += YR2 + px;
		}
		else
		{
			y--;
			py -=twoXR2;
			p +=YR2 + px -py;
		}
	}
	while(px < py);
	
	p = (uint16_t)(YR2 * (x + 0.5) * (x + 0.5) + XR2 * (y - 1) *(y - 1) - XR2 * XR2 + 0.5);
	
	while(y > 0)
	{
		y--;
		py -= twoXR2;
		if (p >0)
		{
			p += XR2 - py;
		}
		else
		{
			x++;
			px += twoYR2;
			p += XR2 -py + px;
		}

		LCD_DrawPixel((Xpos + x), (Ypos + y), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos - x), (Ypos + y), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos + x), (Ypos - y), DrawProp[ActiveLayer].TextColor);
		LCD_DrawPixel((Xpos - x), (Ypos - y), DrawProp[ActiveLayer].TextColor);
	}
}*/
/**
 * [LCD_DrawPolygon 画任意多边型函数]
 * @param Points     [指向各坐标点的指针]
 * @param PointCount [多边形点的个数]
 *note 使用该函数时需要先建立一个 Points类型的数组存放各点的坐标
 *eg ：Point a[]= {{20,20},{50,100},{300,200}};
 */
void LCD_DrawPolygon(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0;

  if(PointCount < 2)
  {
    return;
  }

  LCD_DrawLine(Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);

  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    LCD_DrawLine(X, Y, Points->X, Points->Y);
  }
}
/**
 * [LCD_FillRect 画一个带填充颜色的矩形]
 * @param Xpos   [矩形起点X轴坐标]
 * @param Ypos   [矩形起点Y轴坐标]
 * @param Width  [矩形的宽度]
 * @param Height [矩形的高度]
 */
void LCD_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  uint32_t Xaddr;
  
	Xaddr = (LTDC_Handle.LayerCfg[ActiveLayer].FBStartAdress) + 4 * (Ypos * LCD_GetXSize() + Xpos);
	
	/*填充像素缓存，当前激活的层，缓存地址，像素宽度，像素高度，像素的偏移地址(不只是每行起始的地址，而是从上一行开始，到这一行开始的地址)，当前层的颜色*/
	LL_FillBuffer(ActiveLayer, (uint32_t *)Xaddr, Width, Height, LCD_GetXSize() - Width, DrawProp[ActiveLayer].TextColor);
}
/**
 * [LCD_FillCircle 画一个填充颜色的圆]
 * @param Xpos   [圆心X轴坐标]
 * @param Ypos   [圆心Y轴坐标]
 * @param Radius [圆的半径]
 * note ：中心点画圆法<<计算机图形学>>
 */
void LCD_FillCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
	uint32_t CurX;
	uint32_t CurY;
	int32_t p;

	CurX = 0;
	CurY = Radius;
	p = 5 - 4*Radius; /*p = 5/4 - r 扩大四倍*/

	/*根据圆的对称性将，圆分成8部分，从(0,r)点在第一象限开始绘制*/
	while(CurX <= CurY)
	{

		/*填充两端的部分,根据对称以(-x,y)和(-x,-y)为起点，以2倍的CurX画横线 */
		LCD_DrawHLine((Xpos - CurX), (Ypos - CurY), 2 * CurX);
		LCD_DrawHLine((Xpos - CurX), (Ypos + CurY), 2 * CurX);

		/*填充中间的部分,根据对称以(-y,x)和(-y,-x)为起点，以2倍的CurY画横线 */
		LCD_DrawHLine(Xpos - CurY, Ypos + CurX, 2 *CurY);
      LCD_DrawHLine(Xpos - CurY, Ypos - CurX, 2 *CurY);
		
		if (p < 0)         /*中点画圆法，增量p*/
		{
			p += 8 * CurX + 12;
		}
		else 
		{
			p += 8 * (CurX - CurY) + 20;
			CurY--;
		}

		CurX++;
	}

	/*画外面的圆框*/
	LCD_DrawCircle(Xpos, Ypos, Radius);
}
void DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
{
	uint32_t index = 0, width = 0, height = 0, bit_pixel = 0;
  uint32_t Address;
  uint32_t InputColorMode = 0;

  /* Get bitmap data address offset */
  index = *(__IO uint16_t *) (pbmp + 10);
  index |= (*(__IO uint16_t *) (pbmp + 12)) << 16;

  /* Read bitmap width */
  width = *(uint16_t *) (pbmp + 18);
  width |= (*(uint16_t *) (pbmp + 20)) << 16;

  /* Read bitmap height */
  height = *(uint16_t *) (pbmp + 22);
  height |= (*(uint16_t *) (pbmp + 24)) << 16;

  /* Read bit/pixel */
  bit_pixel = *(uint16_t *) (pbmp + 28);

  /* Set the address */
  Address = LTDC_Handle.LayerCfg[ActiveLayer].FBStartAdress + (((LCD_GetXSize()*Ypos) + Xpos)*(4));

  /* Get the layer pixel format */
  if ((bit_pixel/8) == 4)
  {
    InputColorMode = CM_ARGB8888;
  }
  else if ((bit_pixel/8) == 2)
  {
    InputColorMode = CM_RGB565;
  }
  else
  {
    InputColorMode = CM_RGB888;
  }

  /* Bypass the bitmap header */
  pbmp += (index + (width * (height - 1) * (bit_pixel/8)));

  /* Convert picture to ARGB8888 pixel format */
  for(index=0; index < height; index++)
  {
    /* Pixel format conversion */
    LL_ConvertLineToARGB8888((uint32_t *)pbmp, (uint32_t *)Address, width, InputColorMode);

    /* Increment the source and destination buffers */
    Address+=  (LCD_GetXSize()*4);
    pbmp -= width*(bit_pixel/8);
  }
}
static void LL_ConvertLineToARGB8888(void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode)
{
  /* Configure the DMA2D Mode, Color Mode and output offset */
  hdma2d_eval.Init.Mode         = DMA2D_M2M_PFC;
  hdma2d_eval.Init.ColorMode    = DMA2D_ARGB8888;
  hdma2d_eval.Init.OutputOffset = 0;

  /* Foreground Configuration */
  hdma2d_eval.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d_eval.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d_eval.LayerCfg[1].InputColorMode = ColorMode;
  hdma2d_eval.LayerCfg[1].InputOffset = 0;

  hdma2d_eval.Instance = DMA2D;

  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&hdma2d_eval) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&hdma2d_eval, 1) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&hdma2d_eval, (uint32_t)pSrc, (uint32_t)pDst, xSize, 1) == HAL_OK)
      {
        /* Polling For DMA transfer */
        HAL_DMA2D_PollForTransfer(&hdma2d_eval, 10);
      }
    }
  }
}
/**
 * [LCD_Clear 液晶清屏]
 * @param Color [清屏填充的颜色]
 */
void LCD_Clear(uint32_t Color)
{
	LL_FillBuffer(ActiveLayer, (uint32_t *)(LTDC_Handle.LayerCfg[ActiveLayer].FBStartAdress), LCD_GetXSize(), LCD_GetYSize(), 0, Color);
}

static void LL_FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex)
{
  /* Register to memory mode with ARGB8888 as color Mode */
  hdma2d_eval.Init.Mode         = DMA2D_R2M;
  hdma2d_eval.Init.ColorMode    = DMA2D_ARGB8888;
  hdma2d_eval.Init.OutputOffset = OffLine;

  hdma2d_eval.Instance = DMA2D;

  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&hdma2d_eval) == HAL_OK)
  {
    if(HAL_DMA2D_ConfigLayer(&hdma2d_eval, LayerIndex) == HAL_OK)
    {
      if (HAL_DMA2D_Start(&hdma2d_eval, ColorIndex, (uint32_t)pDst, xSize, ySize) == HAL_OK)
      {
        /* Polling For DMA transfer */
        HAL_DMA2D_PollForTransfer(&hdma2d_eval, 10);
      }
    }
  }
}
/**
  * @brief  This function handles LTDC Error interrupt Handler.
  * @note : Can be surcharged by application code implementation of the function.
  */

__weak void LCD_LTDC_ER_IRQHandler(void)
{
  HAL_LTDC_IRQHandler(&LTDC_Handle);
}
/**
  * @brief  Handles DMA2D interrupt request.
  * @note : Can be surcharged by application code implementation of the function.
  */
/*__weak void BSP_LCD_DMA2D_IRQHandler(void)
{
  HAL_DMA2D_IRQHandler(&hdma2d_eval);
}
*/
/**
  * @brief  Handles DSI interrupt request.
  * @note : Can be surcharged by application code implementation of the function.
  */
__weak void LCD_DSI_IRQHandler(void)
{
  HAL_DSI_IRQHandler(&DSI_Handle);
}


/**
  * @brief  Handles LTDC interrupt request.
  * @note : Can be surcharged by application code implementation of the function.
  */
__weak void LCD_LTDC_IRQHandler(void)
{
  HAL_LTDC_IRQHandler(&LTDC_Handle);
}

