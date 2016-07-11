/*
**************************************************************************************************
*文件：bsp.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-05-27
*描述：本文件主要是对STM32F469-Dis的一些外设进行初始化，并提供一些功能函数，包括
*		LED、按键、外部SDRAM、外部QSPI Flash、RF EERPOM等。
* ************************************************************************************************
*/

#include "bsp.h"

/*
**************************************************************************************************
*										LED port和 pin定义
**************************************************************************************************
 */
uint32_t GPIO_Pin[4]       = {LED1_PIN,LED2_PIN,LED3_PIN,LED4_PIN};
GPIO_TypeDef *GPIO_Port[4] = {LED1_GPIO_PORT,
							  LED2_GPIO_PORT,
						      LED3_GPIO_PORT,
							  LED4_GPIO_PORT,
								};
										/*
**************************************************************************************************
*										Button port和 pin定义
**************************************************************************************************
 */
const uint16_t Button_Pin[1]  = {USER_BUTTON_PIN};
const uint16_t Button_IRQn[1] = {USER_BUTTON_EXTI_IRQn};
GPIO_TypeDef*  Button_Port[1] = {USER_BUTTON_GPIO_PORT };
/*
**************************************************************************************************
*描述：LED初始化函数
*参数：Led 需要初始化的LED
*		 可选的参数有LED1、LED2、LED3、LED4
*返回：无
* ************************************************************************************************
 */
void LED_Init(Led_TypeDef Led)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	/*初始化GPIO结构体变量*/
	GPIO_InitStructure.Pin   = GPIO_Pin[Led];
	GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Pull  = GPIO_PULLUP;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;

	switch(Led)
	{
		case LED1:
			LED1_GPIO_CLK_ENABLE();
			break;
		case LED2:
			LED2_GPIO_CLK_ENABLE();
			break;
		case LED3:
			LED3_GPIO_CLK_ENABLE();
			break;
		case LED4:
			LED4_GPIO_CLK_ENABLE();
			break;
		default:
			break;
	}
	HAL_GPIO_Init(GPIO_Port[Led],&GPIO_InitStructure);
	/*初始LED灯灭*/
	HAL_GPIO_WritePin(GPIO_Port[Led],GPIO_Pin[Led],GPIO_PIN_SET);
	
}
/*
**************************************************************************************************
*描述：LED恢复初始
*参数：Led 需要初始化的LED
*		 可选的参数有LED1、LED2、LED3、LED4
*返回：无
*note：该函数没有禁用GPIO的时钟
* ************************************************************************************************
 */
void LED_DeInit(Led_TypeDef Led)
{
	/*关闭LED*/
	HAL_GPIO_WritePin(GPIO_Port[Led],GPIO_Pin[Led],GPIO_PIN_SET);
	HAL_GPIO_DeInit(GPIO_Port[Led],GPIO_Pin[Led]);
}
/*
**************************************************************************************************
*描述：点亮LED
*参数：Led 需要点亮的LED
*		 可选的参数有LED1、LED2、LED3、LED4
*返回：无
* ************************************************************************************************
 */
void LED_On(Led_TypeDef Led)
{
	HAL_GPIO_WritePin(GPIO_Port[Led],GPIO_Pin[Led],GPIO_PIN_RESET);
}
/*
**************************************************************************************************
*描述：关闭LED
*参数：Led 需要关闭的LED
*		 可选的参数有LED1、LED2、LED3、LED4
*返回：无
* ************************************************************************************************
 */
void LED_Off(Led_TypeDef Led)
{
	HAL_GPIO_WritePin(GPIO_Port[Led],GPIO_Pin[Led],GPIO_PIN_SET);
}
/*
**************************************************************************************************
*描述：闪烁LED
*参数：Led 需要闪烁的LED
*		 可选的参数有LED1、LED2、LED3、LED4
*返回：无
* ************************************************************************************************
 */
void LED_Toggle(Led_TypeDef Led)
{
	HAL_GPIO_TogglePin(GPIO_Port[Led],GPIO_Pin[Led]);
}
/*
**************************************************************************************************
*描述：按键初始化
*参数：Button 		默认参数User_Button
*	   Button_Mode	普通IO模式或者是中断模式
*		 		BUTTON_MODE_GPIO  普通IO模式
*		 		BUTTON_MODE_EXIT	中断模式		  
*返回：无
* ************************************************************************************************
 */
void Button_Init(Button_TypeDef Button,ButtonMode_TypeDef Button_Mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*Enable the Button Clock*/
	BUTTON_GPIO_CLK_ENABLE();
	if (Button_Mode == Button_Mode_GPIO)
	{
		/*配置Button为输入模式*/
		GPIO_InitStructure.Pin   = Button_Pin[Button];
		GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull  = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

		HAL_GPIO_Init(Button_Port[Button],&GPIO_InitStructure);
	}
	if (Button_Mode == Button_Mode_EXIT)
	{
		/*配置Button为外部中断输入模式*/
		GPIO_InitStructure.Pin   = Button;
		GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull  = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

		HAL_GPIO_Init(Button_Port[Button],&GPIO_InitStructure);
		/*使能外部中断，配置为最低优先级*/
		HAL_NVIC_SetPriority((IRQn_Type)(Button_IRQn[Button]),0x0f,0x00);
		HAL_NVIC_EnableIRQ((IRQn_Type)(Button_IRQn[Button]));
	}
}
/*
**************************************************************************************************
*描述：按键初始化
*参数：Button 	默认User_Button	  	  
*返回：无
* ************************************************************************************************
 */
void Button_DeInit(Button_TypeDef Button)
{
	HAL_NVIC_DisableIRQ((IRQn_Type)(Button_IRQn[Button]));
   HAL_GPIO_DeInit(Button_Port[Button], Button_Pin[Button]);
}
/*
**************************************************************************************************
*描述：获取按键的值
*参数：Button 	默认User_Button	  
*返回：按键的值
* ************************************************************************************************
 */
uint32_t Button_GetState(Button_TypeDef Button)
{
  return HAL_GPIO_ReadPin(Button_Port[Button], Button_Pin[Button]);
}
void OTM8009A_IO_Delay(uint32_t delay)
{
  HAL_Delay(delay);
}
 