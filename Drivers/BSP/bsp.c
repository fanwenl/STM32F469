/*
**************************************************************************************************
*文件：bsp.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2016-05-27
*描述：本文件主要是对STM32F469-Dis的一些外设进行初始化，并提供一些功能函数，包括
*		LED、按键、外部SDRAM、外部QSPI Flash等 。
* ************************************************************************************************
*/

#include "bsp.h"



static void I2C1_MspInit(void);
static void I2C2_MspInit(void);
static void I2C1_Init(void);
static void I2C2_Init(void);
static void I2C1_DeInit(void);
static void I2C2_DeInit(void);
static void I2C1_DeMspInit(void);
static void I2C2_DeMspInit(void);


#if defined(USE_IOEXPANDER)
static void            I2C1_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
static uint8_t       I2C1_Read(uint8_t Addr, uint8_t Reg);
#endif /* USE_IOEXPANDER */
static HAL_StatusTypeDef I2C1_ReadMultiple(uint8_t Addr, uint16_t Reg, uint16_t MemAddSize, uint8_t *Buffer, uint16_t Length);
static HAL_StatusTypeDef I2C2_ReadMultiple(uint8_t Addr, uint16_t Reg, uint16_t MemAddSize, uint8_t *Buffer, uint16_t Length);
static HAL_StatusTypeDef I2C1_WriteMultiple(uint8_t Addr, uint16_t Reg, uint16_t MemAddSize, uint8_t *Buffer, uint16_t Length);
static HAL_StatusTypeDef I2C2_WriteMultiple(uint8_t Addr, uint16_t Reg, uint16_t MemAddSize, uint8_t *Buffer, uint16_t Length);
static HAL_StatusTypeDef I2C1_IsDeviceReady(uint16_t DevAddress, uint32_t Trials);
static void              I2C1_Error(uint8_t Addr);
static void              I2C2_Error(uint8_t Addr);

static I2C_HandleTypeDef I2C1_Handle;
static I2C_HandleTypeDef I2C2_Handle;
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
void BSP_LED_Init(Led_TypeDef Led)
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
void BSP_LED_DeInit(Led_TypeDef Led)
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
void BSP_LED_On(Led_TypeDef Led)
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
void BSP_LED_Off(Led_TypeDef Led)
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
void BSP_LED_Toggle(Led_TypeDef Led)
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
void BSP_Button_Init(Button_TypeDef Button,ButtonMode_TypeDef Button_Mode)
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
void BSP_Button_DeInit(Button_TypeDef Button)
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
uint32_t BSP_Button_GetState(Button_TypeDef Button)
{
  return HAL_GPIO_ReadPin(Button_Port[Button], Button_Pin[Button]);
}
/*
**************************************************************************************************
*描述：OTM8009A延时函数
*参数：delay 延时的数值，单位ms	  
*返回：无
* ************************************************************************************************
 */
void OTM8009A_IO_Delay(uint32_t delay)
{
  HAL_Delay(delay);
}
/*
**************************************************************************************************
*描述：I2C1初始化函数
*参数：无	  
*返回：无
* ************************************************************************************************
 */
static void I2C1_Init(void)
{
	if(HAL_I2C_GetState(&I2C1_Handle) == HAL_I2C_STATE_RESET)
	{
		I2C1_Handle.Instance = I2C1;
		/*配置I2C1 Handle*/
		I2C1_Handle.Init.ClockSpeed      = I2C1_SCL_FREQ_KHZ;
		/*快速模式下 Tlow/Thigh = 2*/ 
		I2C1_Handle.Init.DutyCycle       = I2C_DUTYCYCLE_2;
		I2C1_Handle.Init.OwnAddress1     = 0;
		I2C1_Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
		I2C1_Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		I2C1_Handle.Init.OwnAddress2     = 0;
		/*禁止广播呼叫模式*/
		I2C1_Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		/*禁止时钟延长模式,时钟延长具体看手册*/
		I2C1_Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

		I2C1_MspInit();
		HAL_I2C_Init(&I2C1_Handle);
	}

}
/*
**************************************************************************************************
*描述：I2C2恢复默认
*参数：无	  
*返回：无
* ************************************************************************************************
 */
static void I2C1_DeInit(void)
{
	if(HAL_I2C_GetState(&I2C1_Handle) == HAL_I2C_STATE_READY)
	{
		/*复位I2C2的配置*/
		HAL_I2C_DeInit(&I2C1_Handle);
		I2C1_DeMspInit();
	}	
}
/*
**************************************************************************************************
*描述：I2C2初始化函数
*参数：无	  
*返回：无
* ************************************************************************************************
 */
static void I2C2_Init(void)
{
		if(HAL_I2C_GetState(&I2C2_Handle) == HAL_I2C_STATE_RESET)
	{
		I2C2_Handle.Instance = I2C2;
		/*配置I2C1 Handle*/
		I2C2_Handle.Init.ClockSpeed      = I2C2_SCL_FREQ_KHZ;
		/*快速模式下 Tlow/Thigh = 2*/ 
		I2C2_Handle.Init.DutyCycle       = I2C_DUTYCYCLE_2;
		I2C2_Handle.Init.OwnAddress1     = 0;
		I2C2_Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
		I2C2_Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		I2C2_Handle.Init.OwnAddress2     = 0;
		/*禁止广播呼叫模式*/
		I2C2_Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		/*禁止时钟延长模式,时钟延长具体看手册*/
		I2C2_Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

		I2C1_MspInit();
		HAL_I2C_Init(&I2C2_Handle);
	}
}
/*
**************************************************************************************************
*描述：I2C2恢复默认函数
*参数：无	  
*返回：无
* ************************************************************************************************
 */
static void I2C2_DeInit(void)
{
	if(HAL_I2C_GetState(&I2C2_Handle) == HAL_I2C_STATE_READY)
	{
		/*复位I2C2的配置*/
		HAL_I2C_DeInit(&I2C2_Handle);
		I2C1_DeMspInit();
	}	
}
/*
**************************************************************************************************
*描述：I2C1 MSP外设初始化函数
*参数：无	  
*返回：无
* ************************************************************************************************
 */
static void I2C1_MspInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*使能GPIO的时钟*/
	BSP_I2C1_SCL_SDA_GPIO_CLK_ENABLE();
	/*配置I2C1的时钟信号*/
	GPIO_InitStructure.Pin = BSP_I2C1_SCL_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = BSP_I2C1_SCL_SDA_AF;
	HAL_GPIO_Init(BSP_I2C1_SCL_SDA_GPIO_PORT, &GPIO_InitStructure);

	/*配置I2C1的数据信号*/
	GPIO_InitStructure.Pin = BSP_I2C1_SDA_PIN;
	HAL_GPIO_Init(BSP_I2C1_SCL_SDA_GPIO_PORT, &GPIO_InitStructure);
	/*使能I2C1的时钟*/
	BSP_I2C1_CLK_ENABLE();
	/*强制复位I2C1的时钟和外设*/
	BSP_I2C1_FORCE_RESET();
	/*更新I2C1的时钟和外设*/
	BSP_I2C1_RELEASE_RESET();

	/*配置I2C1事件中断*/
	HAL_NVIC_SetPriority(BSP_I2C1_EV_IRQn, 5 ,0);
	HAL_NVIC_EnableIRQ(BSP_I2C1_EV_IRQn);
	/*配置I2C1错误中断*/
	HAL_NVIC_SetPriority(BSP_I2C1_ER_IRQn, 5 ,0);
	HAL_NVIC_EnableIRQ(BSP_I2C1_ER_IRQn);
}
/*
**************************************************************************************************
*描述：I2C2 MSP外设恢复默认
*参数：无	  
*返回：无
* ************************************************************************************************
 */
static void I2C1_DeMspInit(void)
{
	/*禁止I2C1的中断*/
	HAL_NVIC_DisableIRQ(BSP_I2C1_EV_IRQn);
	HAL_NVIC_DisableIRQ(BSP_I2C1_ER_IRQn);
	/*强制复位I2C1的时钟和外设*/
	BSP_I2C1_FORCE_RESET();
	/*禁止I2C1的时钟*/
	BSP_I2C1_CLK_DISABLE();
}
/*
**************************************************************************************************
*描述：I2C2 MSP外设初始化函数
*参数：无	  
*返回：无
* ************************************************************************************************
 */
static void I2C2_MspInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*使能GPIO的时钟*/
	BSP_I2C1_SCL_SDA_GPIO_CLK_ENABLE();
	/*配置I2C1的时钟信号*/
	GPIO_InitStructure.Pin = BSP_I2C2_SCL_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = BSP_I2C2_SCL_SDA_AF;
	HAL_GPIO_Init(BSP_I2C2_SCL_SDA_GPIO_PORT, &GPIO_InitStructure);

	/*配置I2C1的数据信号*/
	GPIO_InitStructure.Pin = BSP_I2C2_SDA_PIN;
	HAL_GPIO_Init(BSP_I2C2_SCL_SDA_GPIO_PORT, &GPIO_InitStructure);
	/*使能I2C1的时钟*/
	BSP_I2C1_CLK_ENABLE();
	/*强制复位I2C1的时钟和外设*/
	BSP_I2C1_FORCE_RESET();
	/*更新I2C1的时钟和外设*/
	BSP_I2C1_RELEASE_RESET();

	/*配置I2C1事件中断*/
	HAL_NVIC_SetPriority(BSP_I2C2_EV_IRQn, 5 ,0);
	HAL_NVIC_EnableIRQ(BSP_I2C2_EV_IRQn);
	/*配置I2C1错误中断*/
	HAL_NVIC_SetPriority(BSP_I2C2_ER_IRQn, 5 ,0);
	HAL_NVIC_EnableIRQ(BSP_I2C2_ER_IRQn);
}
/*
**************************************************************************************************
*描述：I2C2 MSP外设恢复默认函数
*参数：无	  
*返回：无
* ************************************************************************************************
 */
static void I2C2_DeMspInit(void)
{
	/*禁止I2C1的中断*/
	HAL_NVIC_DisableIRQ(BSP_I2C2_EV_IRQn);
	HAL_NVIC_DisableIRQ(BSP_I2C2_ER_IRQn);
	/*强制复位I2C1的时钟和外设*/
	BSP_I2C2_FORCE_RESET();
	/*禁止I2C1的时钟*/
	BSP_I2C2_CLK_DISABLE();	
}
/*
**************************************************************************************************
*描述：I2C1	写单个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存器地址
*		 Value写入数据的值	  
*返回：返回操作结果
* ************************************************************************************************
 */
static uint8_t I2C1_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
	HAL_StatusTypeDef status = HAL_OK;
	status = HAL_I2C_Mem_Write(&I2C1_Handle, 
										Addr, 
										(uint16_t)Reg, 
										I2C_MEMADD_SIZE_8BIT, 
										&Value, 
										1, 
										100);
	if(status != HAL_OK)
	{
		I2C1_Error(Addr);
	}
	return status;
}
/*
**************************************************************************************************
*描述：I2C1	读单个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存器地址	  
*返回：返回读出数据
* ************************************************************************************************
 */
static uint8_t I2C1_Read(uint8_t Addr, uint8_t Reg)
{
	HAL_StatusTypeDef  status = HAL_OK;
	uint8_t Value;
	status = HAL_I2C_Mem_Read(&I2C1_Handle, 
										Addr, 
										(uint16_t)Reg, 
										I2C_MEMADD_SIZE_8BIT, 
										&Value, 
										1, 
										100);
	if(status != HAL_OK)
	{
		I2C1_Error(Addr);
	}
	return Value;
}
/*
**************************************************************************************************
*描述：I2C1	读多个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存器地址	
*		 MemAddsize  数据的大小
*		 Buffer 数据缓存的地址
*		 Length 数据的长度  
*返回：返回操作结果
* ************************************************************************************************
 */
static HAL_StatusTypeDef I2C1_ReadMultiple(uint8_t Addr, 
														 uint16_t Reg,
														 uint16_t MemAddSize, 
														 uint8_t *Buffer, 
														 uint16_t Length)
{
	HAL_StatusTypeDef  status = HAL_OK;
	status = HAL_I2C_Mem_Read(&I2C1_Handle, 
										Addr, 
										(uint16_t)Reg, 
										MemAddSize, 
										Buffer, 
										Length, 
										100);
	if(status != HAL_OK)
	{
		I2C1_Error(Addr);
	}
	return status;
}
/*
**************************************************************************************************
*描述：I2C2	读多个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存器地址	
*		 MemAddsize  数据的大小
*		 Buffer 数据缓存的地址
*		 Length 数据的长度  
*返回：返回操作结果
* ************************************************************************************************
 */
static HAL_StatusTypeDef I2C2_ReadMultiple(uint8_t Addr, 
														 uint16_t Reg, 
														 uint16_t MemAddSize, 
														 uint8_t *Buffer, 
														 uint16_t Length)
{
	HAL_StatusTypeDef  status = HAL_OK;
	status = HAL_I2C_Mem_Read(&I2C2_Handle, 
										Addr, 
										(uint16_t)Reg, 
										MemAddSize, 
										Buffer, 
										Length, 
										100);
	if(status != HAL_OK)
	{
		I2C1_Error(Addr);
	}
	return status;
}
/*
**************************************************************************************************
*描述：I2C1	写多个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存器地址	
*		 MemAddsize  数据的大小
*		 Buffer 数据缓存的地址
*		 Length 数据的长度  
*返回：返回操作结果
* ************************************************************************************************
 */
static HAL_StatusTypeDef I2C1_WriteMultiple(uint8_t Addr, 
														  uint16_t Reg, 
														  uint16_t MemAddSize, 
														  uint8_t *Buffer, 
														  uint16_t Length)
{
	HAL_StatusTypeDef  status = HAL_OK;
	status = HAL_I2C_Mem_Write(&I2C1_Handle, 
										Addr, 
										(uint16_t)Reg, 
										MemAddSize, 
										Buffer, 
										Length, 
										100);
	if(status != HAL_OK)
	{
		I2C1_Error(Addr);
	}
	return status;
}
/*
**************************************************************************************************
*描述：I2C2	写多个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存器地址	
*		 MemAddsize  数据的大小
*		 Buffer 数据缓存的地址
*		 Length 数据的长度  
*返回：返回操作结果
* ************************************************************************************************
 */
static HAL_StatusTypeDef I2C2_WriteMultiple(uint8_t Addr,
														  uint16_t Reg,
														  uint16_t MemAddSize,
														  uint8_t *Buffer,
														  uint16_t Length)
{
	HAL_StatusTypeDef  status = HAL_OK;
	status = HAL_I2C_Mem_Write(&I2C2_Handle, 
										Addr, 
										(uint16_t)Reg, 
										MemAddSize, 
										Buffer, 
										Length, 
										100);
	if(status != HAL_OK)
	{
		I2C1_Error(Addr);
	}
	return status;
}
/*
**************************************************************************************************
*描述：I2C1	写多个数据函数
*参数：DevAddress I2C设备地址
*		 Trials   尝试的次数 
*返回：返回操作结果
* ************************************************************************************************
 */
static HAL_StatusTypeDef I2C1_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{
	return (HAL_I2C_IsDeviceReady(&I2C2_Handle, DevAddress, Trials, 1000));
}
/*
**************************************************************************************************
*描述：I2C2	写多个数据函数
*参数：DevAddress I2C设备地址
*		 Trials   尝试的次数 
*返回：返回操作结果
* ************************************************************************************************
 */
static HAL_StatusTypeDef I2C2_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{
	return (HAL_I2C_IsDeviceReady(&I2C2_Handle, DevAddress, Trials, 1000));
}
/*
**************************************************************************************************
*描述：I2C1	错误处理函数
*参数：Addr I2C设备地址
*返回：无
* ************************************************************************************************
 */
static void I2C1_Error(uint8_t Addr)
{
	HAL_I2C_DeInit(&I2C1_Handle);
	I2C1_Init();
}
/*
**************************************************************************************************
*描述：I2C1	错误处理函数
*参数：Addr I2C设备地址
*返回：无
* ************************************************************************************************
 */
static void I2C2_Error(uint8_t Addr)
{
	HAL_I2C_DeInit(&I2C2_Handle);
	I2C2_Init();
}
/*
**********************************************************************************************************
*												TS接口函数
* ********************************************************************************************************
 */
/*
**************************************************************************************************
*描述：触摸IO初始化函数
*参数：无
*返回：无
* ************************************************************************************************
 */
void TS_IO_Init(void)
{
	I2C1_Init();
}
/*
**************************************************************************************************
*描述：触摸IO复位函数
*参数：无
*返回：无
* ************************************************************************************************
 */
void TS_IO_DeInit(void)
{
	I2C1_DeInit();
}
/*
**************************************************************************************************
*描述：触摸IO写单个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存地址
*		 Value 数据的值
*返回：无
* ************************************************************************************************
 */
uint8_t TS_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
	return I2C1_Write(Addr, Reg, Value);
}
/*
**************************************************************************************************
*描述：触摸IO读单个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存地址
*返回：返回读取数据值
* ************************************************************************************************
 */
uint8_t TS_IO_Read(uint8_t Addr, uint8_t Reg)
{
	return I2C1_Read(Addr, Reg);
}
/*
**************************************************************************************************
*描述：触摸IO读多个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存地址
*		 MemAddsize 数据的大小
*		 Buffer  数据缓存地址
*		 Length  数据长度
*返回：无
* ************************************************************************************************
 */
uint8_t TS_IO_ReadMultiple(uint8_t Addr, uint16_t Reg, uint8_t *Buffer, uint16_t Length)
{
	return I2C1_ReadMultiple(Addr, Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
}
/*
**************************************************************************************************
*描述：触摸IO写多个数据函数
*参数：Addr I2C设备地址
*		 Reg  寄存地址
*		 MemAddsize 数据的大小
*		 Buffer  数据缓存地址
*		 Length  数据长度
*返回：无
* ************************************************************************************************
 */
uint8_t TS_IO_WriteMultiple(uint8_t Addr, uint16_t Reg, uint8_t *Buffer, uint16_t Length)
{
	 return I2C1_WriteMultiple(Addr, Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
}
/*
**************************************************************************************************
*描述：触摸IO延时函数
*参数：Delay 延时参数单位ms
*返回：无
* ************************************************************************************************
 */
void TS_IO_Delay(uint32_t Delay)
{
	HAL_Delay(Delay);
}
/*
**********************************************************************************************************
*												Audio接口函数
* ********************************************************************************************************
 */

/*
**********************************************************************************************************
*												MFRC522-SPI接口函数
* ********************************************************************************************************
 */
