#ifndef _DISPLAYJPEG_H
#define _DISPLAYJPEG_H

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
//STemwin JPEGͼƬ��ʾ 
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/3/30
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


//ʹ��GUI_JPEG_Draw()��������BMPͼƬ�Ļ�
//ͼƬ�Ǽ��ص�RAM�еģ���˲��ܴ���JEGPMEMORYSIZE
//ע�⣺��ʾBMPͼƬʱ�ڴ�����ʹ�õ�EMWIN���ڴ����뺯�������
//JPEGMEMORYSIZE���ܴ������Ǹ�EMWIN������ڴ�ش�С
#define JPEGMEMORYSIZE	(8*1024*1024)	//ͼƬ��С������10M

//����������ص�RAM�е�JPEGͼƬʱ��ͼƬÿ�е��ֽ���
#define JPEGPERLINESIZE	2*1024		

int displyjpeg(U8 *JPEGFileName,U8 mode,U32 x,U32 y,int member,int denom);
int displayjpegex(U8 *JPEGFileName,U8 mode,U32 x,U32 y,int member,int denom);
void jpegdisplay_demo(void);

#ifdef __cplusplus
}
#endif

#endif
