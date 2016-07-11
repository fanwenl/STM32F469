/*
*************************************************************************************************
*文件：bsp.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-05-27
*描述：本文件主要是对STM32F469-Dis外设的定义，包括LED、按键、外部SDRAM、外部QSPI Flash、RF ERPOM等。
**************************************************************************************************
*/
#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif /*cpluspluse*/

#include "stm32f4xx_hal.h"

/*
****************************************************************************************************
*												LED灯枚举
* **************************************************************************************************
 */
typedef enum
{
	LED1 = 0,
	LED_Green = LED1,
	LED2 = 1,
	LED_Orange = LED2,
	LED3 = 2,
	LED_Red = LED3,
	LED4 = 3,
	LED_Blue = LED4,
} Led_TypeDef;
/*
****************************************************************************************************
*												按键结构定义
* **************************************************************************************************
 */
typedef enum
{
	User_Button = 0
} Button_TypeDef;

/*按键模式选择*/
typedef enum
{
	Button_Mode_GPIO = 0,
	Button_Mode_EXIT = 1
} ButtonMode_TypeDef;
/*按键值定义*/
typedef enum
{
	PB_Set = 0,
	PB_ReSet = !PB_Set
} ButtonValue_TypeDef;
/*
****************************************************************************************************
*												函数申明
* **************************************************************************************************
 */
void 	LED_Init(Led_TypeDef Led);
void 	LED_DeInit(Led_TypeDef Led);	
void 	LED_On(Led_TypeDef Led);
void 	LED_Off(Led_TypeDef Led);
void 	LED_Toggle(Led_TypeDef);
void 	Button_Init(Button_TypeDef Button,ButtonMode_TypeDef Button_Mode);
void 	Button_DeInit(Button_TypeDef Button);
uint32_t Button_GetState(Button_TypeDef Button);
void OTM8009A_IO_Delay(uint32_t delay);
/*
****************************************************************************************************
*												LED定义
* **************************************************************************************************
 */
/*定义LED灯的GPIO_PIN*/
#define LED1_PIN					((uint32_t)GPIO_PIN_6)
#define LED2_PIN					((uint32_t)GPIO_PIN_4)
#define LED3_PIN					((uint32_t)GPIO_PIN_5)
#define LED4_PIN					((uint32_t)GPIO_PIN_3)
/*定义LED灯的GPIO_PORT*/
#define LED1_GPIO_PORT				((GPIO_TypeDef *)GPIOG)
#define LED2_GPIO_PORT				((GPIO_TypeDef *)GPIOD)
#define LED3_GPIO_PORT				((GPIO_TypeDef *)GPIOD)
#define LED4_GPIO_PORT				((GPIO_TypeDef *)GPIOK)
/*定义LED灯的时钟*/
#define LED1_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOG_CLK_ENABLE()
#define LED1_GPIO_CLK_DISABLE()	__HAL_RCC_GPIOG_CLK_DISABLE()
#define LED2_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOD_CLK_ENABLE()
#define LED2_GPIO_CLK_DISABLE()	__HAL_RCC_GPIOD_CLK_DISABLE()
#define LED3_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOD_CLK_ENABLE()
#define LED3_GPIO_CLK_DISABLE()	__HAL_RCC_GPIOD_CLK_DISABLE()
#define LED4_GPIO_CLK_ENABLE()	__HAL_RCC_GPIOK_CLK_ENABLE()
#define LED4_GPIO_CLK_DISABLE()	__HAL_RCC_GPIOK_CLK_DISABLE()

/*
****************************************************************************************************
*												按键定义
* **************************************************************************************************
 */
#define USER_BUTTON_PIN                     GPIO_PIN_0
#define USER_BUTTON_GPIO_PORT             	((GPIO_TypeDef *)GPIOA)
#define USER_BUTTON_EXTI_IRQn             	EXTI0_IRQn

#define BUTTON_GPIO_CLK_ENABLE()            __HAL_RCC_GPIOA_CLK_ENABLE()
#ifdef __cplusplus
}
#endif /*cplusplus*/

#endif /*__BSP_H*/