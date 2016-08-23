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
void 	BSP_LED_Init(Led_TypeDef Led);
void 	BSP_LED_DeInit(Led_TypeDef Led);	
void 	BSP_LED_On(Led_TypeDef Led);
void 	BSP_LED_Off(Led_TypeDef Led);
void 	BSP_LED_Toggle(Led_TypeDef Led);
void 	BSP_Button_Init(Button_TypeDef Button,ButtonMode_TypeDef Button_Mode);
void 	BSP_Button_DeInit(Button_TypeDef Button);
uint32_t BSP_Button_GetState(Button_TypeDef Button);
void OTM8009A_IO_Delay(uint32_t delay);
/*
****************************************************************************************************
*												LED定义
* **************************************************************************************************
 */
/*定义LED灯的GPIO_PIN*/
#define LED1_PIN						((uint32_t)GPIO_PIN_6)
#define LED2_PIN						((uint32_t)GPIO_PIN_4)
#define LED3_PIN						((uint32_t)GPIO_PIN_5)
#define LED4_PIN						((uint32_t)GPIO_PIN_3)
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
#define USER_BUTTON_PIN					GPIO_PIN_0
#define USER_BUTTON_GPIO_PORT			((GPIO_TypeDef *)GPIOA)
#define USER_BUTTON_EXTI_IRQn			EXTI0_IRQn
#define BUTTON_GPIO_CLK_ENABLE()		__HAL_RCC_GPIOA_CLK_ENABLE()

#define TS_INT_PIN						((uint32_t)GPIO_PIN_5)
#define TS_INT_GPIO_PORT				((GPIO_TypeDef*)GPIOJ)
#define TS_INT_GPIO_CLK_ENABLE()		__HAL_RCC_GPIOJ_CLK_ENABLE()
#define TS_INT_GPIO_CLK_DISABLE()	__HAL_RCC_GPIOJ_CLK_DISABLE()
#define TS_INT_EXTI_IRQn				EXTI9_5_IRQn

/*
****************************************************************************************************
*												I2C1定义
* **************************************************************************************************
 */
/**
  * @brief TouchScreen FT6206 Slave I2C address
  */
#define TS_I2C_ADDRESS                   ((uint16_t)0x54)


/**
  * @brief Audio I2C Slave address
  */
#define AUDIO_I2C_ADDRESS                ((uint16_t)0x94)

/**
  * @brief EEPROM I2C Slave address 1
  */
#define EEPROM_I2C_ADDRESS_A01           ((uint16_t)0xA0)

/**
  * @brief EEPROM I2C Slave address 2
  */
#define EEPROM_I2C_ADDRESS_A02           ((uint16_t)0xA6)

/**
  * @brief I2C clock speed configuration (in Hz)
  * WARNING:
  * Make sure that this define is not already declared in other files
  * It can be used in parallel by other modules.
  */
#ifndef I2C1_SCL_FREQ_KHZ
#define I2C1_SCL_FREQ_KHZ                  400000 /*!< f(I2C_SCL) = 400 kHz */
#endif /* I2C1_SCL_FREQ_KHZ */

/*I21C的时钟资源定义*/
#define BSP_I2C1                             I2C1
#define BSP_I2C1_CLK_ENABLE()                __HAL_RCC_I2C1_CLK_ENABLE()
#define BSP_I2C1_CLK_DISABLE()				 __HAL_RCC_I2C1_CLK_DISABLE()
//#define BSP_DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define BSP_I2C1_SCL_SDA_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
#define BSP_I2C1_FORCE_RESET()               __HAL_RCC_I2C1_FORCE_RESET()
#define BSP_I2C1_RELEASE_RESET()             __HAL_RCC_I2C1_RELEASE_RESET()

/*I2C1的引脚定义，用于触摸*/
#define BSP_I2C1_SCL_PIN                     GPIO_PIN_8 /*!< PB8 */
#define BSP_I2C1_SCL_SDA_GPIO_PORT           GPIOB
#define BSP_I2C1_SCL_SDA_AF                  GPIO_AF4_I2C1
#define BSP_I2C1_SDA_PIN                     GPIO_PIN_9 /*!< PB9 */

/*I2C1中断定义*/
#define BSP_I2C1_EV_IRQn                     I2C1_EV_IRQn
#define BSP_I2C1_ER_IRQn                     I2C1_ER_IRQn
/*
****************************************************************************************************
*												I2C2定义
* **************************************************************************************************
 */
/**
  * @brief I2C2 clock speed configuration (in Hz)
  * WARNING:
  * Make sure that this define is not already declared in other files
  * It can be used in parallel by other modules.
  */
#ifndef I2C2_SCL_FREQ_KHZ
#define I2C2_SCL_FREQ_KHZ                  100000 /*!< f(I2C2_SCL) < 100 kHz */
#endif /* I2C2_SCL_FREQ_KHZ */

/*I2C2的时钟资源定义*/
#define BSP__I2C2                             I2C2
#define BSP__I2C2_CLK_ENABLE()                __HAL_RCC_I2C2_CLK_ENABLE()
  #define BSP_I2C1_CLK_DISABLE()			  __HAL_RCC_I2C1_CLK_DISABLE()
#define BSP__I2C2_SCL_SDA_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOH_CLK_ENABLE()
#define BSP__I2C2_FORCE_RESET()               __HAL_RCC_I2C2_FORCE_RESET()
#define BSP__I2C2_RELEASE_RESET()             __HAL_RCC_I2C2_RELEASE_RESET()

/*I2C2引脚定义*/
#define BSP_I2C2_SCL_PIN                     GPIO_PIN_4 /*!< PH4 */
#define BSP_I2C2_SCL_SDA_GPIO_PORT           GPIOH
#define BSP_I2C2_SCL_SDA_AF                  GPIO_AF4_I2C2
#define BSP_I2C2_SDA_PIN                     GPIO_PIN_5 /*!< PH5 */

/*I2C2的中断定义*/
#define BSP_I2C2_EV_IRQn                     I2C2_EV_IRQn
#define BSP_I2C2_ER_IRQn                     I2C2_ER_IRQn



#ifdef __cplusplus
}
#endif /*cplusplus*/

#endif /*__BSP_H*/