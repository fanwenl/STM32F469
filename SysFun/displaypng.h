#ifndef __DISPLAYPNG_H
#define __DISPLAYPNG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "malloc.h"
#include "ff.h"
#include "bsp.h"
#include "GUI.h"
#include "LCDConf.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//STemwin PNGͼƬ��ʾ 
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/3/30
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////////	

//ʹ��GUI_PNG_Draw()��������BMPͼƬ�Ļ�
//ͼƬ�Ǽ��ص�RAM�еģ���˲��ܴ���PNGMEMORYSIZE
//ע�⣺��ʾPNGͼƬʱ�ڴ�����ʹ�õ�EMWIN���ڴ����뺯�������
//PNGMEMORYSIZE���ܴ������Ǹ�EMWIN������ڴ�ش�С
#define PNGMEMORYSIZE	(8*1024*1024)	//ͼƬ��С������8M

//����������ص�RAM�е�PNGͼƬʱ��ͼƬÿ�е��ֽ���
#define PNGPERLINESIZE	5*1024	
	
int displaypng(char *PNGFileName,U8 mode,U32 x,U32 y);
int displaypngex(char *PNGFileName,U8 mode,U32 x,U32 y);
void pngdisplay_demo(void);

#ifdef __cplusplus
}
#endif

#endif
