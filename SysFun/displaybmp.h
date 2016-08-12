#ifndef __DISPLAYBMP_H
#define __DISPLAYBMP_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "malloc.h"
#include "ff.h"
#include "bsp.h"
#include "GUI.h"
#include "LCDConf.h"

#define BMPMEMORYSIZE	(8*1024*1024)	//图片大小不大于8M

//绘制无需加载到RAM中的BMP图片时，图片每行的字节数
#define BMPPERLINESIZE	2*1024		

extern LCD_LayerPropTypedef     layer_prop[GUI_NUM_LAYERS];
	
int dispbmp(U8 *BMPFileName,U8 mode,U32 x,U32 y,int member,int denom);
int dispbmpex(U8 *BMPFileName,U8 mode,U32 x,U32 y,int member,int denom);
void emwinbmp_new_pathname(U8 *pname);
void create_bmppicture(U8 *filename,int x0,int y0,int Xsize,int Ysize);
void bmpdisplay_demo(void);

#ifdef __cplusplus
}
#endif

#endif /*DSIPLAYBMP*/