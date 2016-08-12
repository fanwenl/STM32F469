#include "displaybmp.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//STemwin BMP图片显示 
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2016/3/30
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

static FIL BMPFile;
//static FIL ScrSortFile;                 //屏幕截图文件	  
//static char bmpBuffer[BMPPERLINESIZE];	//定义bmp图片的缓存
/*******************************************************************
*
*       Static functions
*
********************************************************************
*/
/*********************************************************************
*
*       BmpGetData
*
* Function description
*   This routine is called by GUI_BMP_DrawEx(). The routine is responsible
*   for setting the data pointer to a valid data location with at least
*   one valid byte.
*
* Parameters:
*   p           - Pointer to application defined data.
*   NumBytesReq - Number of bytes requested.
*   ppData      - Pointer to data pointer. This pointer should be set to
*                 a valid location.
*   StartOfFile - If this flag is 1, the data pointer should be set to the
*                 beginning of the data stream.
*
* Return value:
*   Number of data bytes available.
*/
static U32 BmpGetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	static int readaddress=0;
	char bmpBuffer[BMPPERLINESIZE];
	FIL * phFile;
	UINT NumBytesRead;
	#if SYSTEM_SUPPORT_OS
		CPU_SR_ALLOC();
	#endif
	
	phFile = (FIL *)p;
	
	/*最多只读取sizeof(bmpBuffer)大小的数据*/
	if (NumBytesReq > sizeof(bmpBuffer)) 
	{
		NumBytesReq = sizeof(bmpBuffer);
	}

	if(Off == 1) readaddress = 0;
	else readaddress=Off;
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_ENTER();	//临界区
	#endif
	/*移动指针到应该读取的位置*/	
	f_lseek(phFile,readaddress); 
	
	//读取数据到缓冲区中
	f_read(phFile,bmpBuffer,NumBytesReq,&NumBytesRead);
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_EXIT();	//退出临界区 
	#endif
	*ppData = (U8 *)bmpBuffer;
	return NumBytesRead;//返回读取到的字节数
}

//在指定位置显示加载到RAM中的BMP图片
//BMPFileName:图片在SD卡或者其他存储设备中的路径(需文件系统支持！)
//			指向文件的名称，所以定义为一个指针，输入的参数为""
//mode:显示模式
//		0 在指定位置显示，由参数x,y确定显示位置
//		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
//x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
//y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
//member:  缩放比例的分子项
//denom:缩放比例的分母项
//返回值:0 显示正常,其他 失败
int dispbmp(U8 *BMPFileName,U8 mode,U32 x,U32 y,int member,int denom)
{
	U16 bread;
	char *bmpbuffer;
	char result;
	int XSize,YSize;
	float Xflag,Yflag;
	
	#if SYSTEM_SUPPORT_OS
		CPU_SR_ALLOC();
	#endif

	result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);	//打开文件
	//文件打开错误或者文件大于BMPMEMORYSIZE
	if((result != FR_OK) || (BMPFile.fsize>BMPMEMORYSIZE)) 	return 1;

	bmpbuffer = mymalloc(SRAMEX,BMPFile.fsize);//申请内存
	if(bmpbuffer == NULL) return 2;
	
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_ENTER();	//临界区
	#endif
		
	result = f_read(&BMPFile,bmpbuffer,BMPFile.fsize,(UINT *)&bread); //读取数据
	if(result != FR_OK) return 3;
	
	#if SYSTEM_SUPPORT_OS
		OS_CRITICAL_EXIT();//退出临界区
	#endif
	
	f_close(&BMPFile);				//关闭BMPFile文件

	switch(mode)
	{
		case 0:	//在指定位置显示图片
			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
			{
				GUI_BMP_Draw(bmpbuffer,x,y);	//在指定位置显示BMP图片
			}else //否则图片需要缩放
			{
				GUI_BMP_DrawScaled(bmpbuffer,x,y,member,denom);
			}
			break;
		case 1:	//在LCD中间显示图片
			XSize = GUI_BMP_GetXSize(bmpbuffer);	//获取图片的X轴大小
			YSize = GUI_BMP_GetYSize(bmpbuffer);	//获取图片的Y轴大小
			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
			{
				//在LCD中间显示图片
				GUI_BMP_Draw(bmpbuffer,(LCD_GetXSize() - XSize)/2-1,(LCD_GetYSize() - YSize)/2-1);
			}else //否则图片需要缩放
			{
				/*计算缩放后的图片的大小，以及坐标点*/
				Xflag = (float)XSize*((float)member/(float)denom);
				Yflag = (float)YSize*((float)member/(float)denom);
				XSize = (LCD_GetXSize()-(int)Xflag)/2-1;
				YSize = (LCD_GetYSize()-(int)Yflag)/2-1;
				GUI_BMP_DrawScaled(bmpbuffer,XSize,YSize,member,denom);
			}
			break;
	}
	myfree(SRAMEX,bmpbuffer);		//释放内存
	return 0;
}

//在指定位置显示无需加载到RAM中的BMP图片(需文件系统支持！对于小RAM，推荐使用此方法！)
//BMPFileName:图片在SD卡或者其他存储设备中的路径
//mode:显示模式
//		0 在指定位置显示，有参数x,y确定显示位置
//		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
//x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
//y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
//member:  缩放比例的分子项
//denom:缩放比例的分母项
//返回值:0 显示正常,其他 失败
int dispbmpex(U8 *BMPFileName,U8 mode,U32 x,U32 y,int member,int denom)
{
	char result;
	int XSize,YSize;
	float Xflag,Yflag;
	
	result = f_open(&BMPFile,(const TCHAR*)BMPFileName,FA_READ);	//打开文件
	//文件打开错误
	if(result != FR_OK) 	return 1;
		
	switch(mode)
	{
		case 0:	//在指定位置显示图片
			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
			{
				/*&BMPFile是传递给BmpGetData的参数*/
				GUI_BMP_DrawEx((GUI_GET_DATA_FUNC *)BmpGetData,&BMPFile,x,y);//在指定位置显示BMP图片
			}else //否则图片需要缩放
			{
				GUI_BMP_DrawScaledEx((GUI_GET_DATA_FUNC *)BmpGetData,&BMPFile,x,y,member,denom);
			}
			break;
		case 1:	//在LCD中间显示图片
			XSize = GUI_BMP_GetXSizeEx((GUI_GET_DATA_FUNC *)BmpGetData,&BMPFile);	//获取图片的X轴大小
			YSize = GUI_BMP_GetYSizeEx((GUI_GET_DATA_FUNC *)BmpGetData,&BMPFile);	//获取图片的Y轴大小
			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
			{
				//在LCD中间显示图片
				GUI_BMP_DrawEx((GUI_GET_DATA_FUNC *)BmpGetData,&BMPFile,(LCD_GetXSize()-XSize)/2-1,(LCD_GetYSize()-YSize)/2-1);
			}else //否则图片需要缩放
			{
				Xflag = (float)XSize*((float)member/(float)denom);
				Yflag = (float)YSize*((float)member/(float)denom);
				XSize = (LCD_GetXSize()-(int)Xflag)/2-1;
				YSize = (LCD_GetYSize()-(int)Yflag)/2-1;
				GUI_BMP_DrawScaledEx((GUI_GET_DATA_FUNC *)BmpGetData,&BMPFile,XSize,YSize,member,denom);
			}
			break;
	}
	f_close(&BMPFile);		//关闭BMPFile文件
	return 0;
}

/**
 * 此函数被GUI_BMP_Serialize()调用，用来向文件写入字节
 * @param Data [写入的数据]
 * @param p    [文件参数指针]
 */
static void _WriteByte2File(U8 Data, void * p) 
{
	UINT nWritten;
	f_write((FIL*)p,&Data,1,&nWritten);
}

//得到一个文件名
//pname:得到的文件名
void emwinbmp_new_pathname(U8 *pname)
{
    U8 res;
    U16 index=0;
    FIL ScrSortFile; 		
    while(index<0XFFFF)
    {
        sprintf((char*)pname,"0:/SCREENSHORT/屏幕截图%05d.bmp",index);
        res=f_open(&ScrSortFile,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
        if(res==FR_NO_FILE)break;   //改文件名不存在=正是我们需要的
        else if(res==FR_OK) f_close(&ScrSortFile);//如果打开成功，就关闭掉
        index++;
    }
}
//生成一个BMP图片，保存到SD卡中,实现屏幕截图功能！
//*filepath:文件路径
//x0:创建BMP文件的X起始位置
//y0:创建BMP文件的Y起始位置
//Xsize:X的大小
//Ysize:Y的大小
void create_bmppicture(U8 *filename,int x0,int y0,int Xsize,int Ysize)
{
	static FIL hFile;
	//创建一个文件，路径为filename,如果文件原本就存在的话会被新建的文件覆盖掉！
	f_open(&hFile,(const TCHAR*)filename,FA_READ|FA_WRITE|FA_CREATE_ALWAYS);
	 /*hFile 是传递到_WriteByte2File的参数*/
	GUI_BMP_SerializeEx(_WriteByte2File,x0,y0,Xsize,Ysize,&hFile);
	f_close(&hFile);	//关闭文件
}	

void bmpdisplay_demo(void)
{
	GUI_SetBkColor(GUI_WHITE);
	GUI_SetColor(GUI_RED);
//	GUI_SetFont(&GUI_FontHZ16);
	GUI_Clear();
	
//	while(1)
	{
//		GUI_DispStringHCenterAt("在指定位置显示一张加载到RAM中的BMP图片",400,5);
		dispbmp("0:111.bmp",0,0,0,1,1);
		GUI_Delay(10);
		dispbmp("0:app_70.bmp",0,40,40,1,1);
		GUI_Delay(10);
//		GUI_Clear();
		dispbmp("0:game_70.bmp",0,150,40,1,1);
		GUI_Delay(10);
		dispbmp("0:ebook_70.bmp",0,260,40,1,1);
		GUI_Delay(10);
		dispbmp("0:ebook_70.bmp",0,370,40,1,1);
		GUI_Delay(10);

		while(1);
	
/*		GUI_DispStringHCenterAt("在LCD中间显示一张缩小1/2的加载到RAM中的BMP图片",400,5);
		dispbmp("0:/PICTURE/BMP/ALIENTEKLOGO.bmp",1,0,	0,	1,2);
		GUI_Delay(1000);
		GUI_Clear();
	
		GUI_DispStringHCenterAt("在LCD中间显示一张扩大4/3倍的加载到RAM中的BMP图片",400,5);
		dispbmp("0:/PICTURE/BMP/ALIENTEKLOGO.bmp",1,0,	0,	4,3);
		GUI_Delay(1000);
		GUI_Clear();
		
		GUI_DispStringHCenterAt("在指定位置显示一张无需加载的BMP图片",400,5);
		dispbmpex("0:/PICTURE/BMP/ALIENTEKLOGO.bmp",0,10,	78,	1,1);
		GUI_Delay(1000);
		GUI_Clear();
	
	
		GUI_DispStringHCenterAt("在LCD中间显示一张缩小1/2的无需加载的BMP图片",400,5);
		dispbmpex("0:/PICTURE/BMP/ALIENTEKLOGO.bmp",1,0,	0,	1,2);
		GUI_Delay(1000);
		GUI_Clear();
	
		GUI_DispStringHCenterAt("在LCD中间显示一张扩大4/3倍的无需加载的BMP图片",400,5);
		dispbmpex("0:/PICTURE/BMP/ALIENTEKLOGO.bmp",1,0,	0,	4,3);
		GUI_Delay(1000);
		GUI_Clear();
*/	}
}
