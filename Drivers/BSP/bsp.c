/*
**************************************************************************************************
*�ļ���bsp.c
*���ߣ�fanwenl_
*�汾��V0.0.1
*���ڣ�2016-05-27
*���������ļ���Ҫ�Ƕ�STM32F469-Dis��һЩ������г�ʼ�������ṩһЩ���ܺ���������
*		LED���������ⲿSDRAM���ⲿQSPI Flash�� ��
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
*										LED port�� pin����
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
*										Button port�� pin����
**************************************************************************************************
 */
const uint16_t Button_Pin[1]  = {USER_BUTTON_PIN};
const uint16_t Button_IRQn[1] = {USER_BUTTON_EXTI_IRQn};
GPIO_TypeDef*  Button_Port[1] = {USER_BUTTON_GPIO_PORT };
/*
**************************************************************************************************
*������LED��ʼ������
*������Led ��Ҫ��ʼ����LED
*		 ��ѡ�Ĳ�����LED1��LED2��LED3��LED4
*���أ���
* ************************************************************************************************
 */
void BSP_LED_Init(Led_TypeDef Led)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	/*��ʼ��GPIO�ṹ�����*/
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
	/*��ʼLED����*/
	HAL_GPIO_WritePin(GPIO_Port[Led],GPIO_Pin[Led],GPIO_PIN_SET);
	
}
/*
**************************************************************************************************
*������LED�ָ���ʼ
*������Led ��Ҫ��ʼ����LED
*		 ��ѡ�Ĳ�����LED1��LED2��LED3��LED4
*���أ���
*note���ú���û�н���GPIO��ʱ��
* ************************************************************************************************
 */
void BSP_LED_DeInit(Led_TypeDef Led)
{
	/*�ر�LED*/
	HAL_GPIO_WritePin(GPIO_Port[Led],GPIO_Pin[Led],GPIO_PIN_SET);
	HAL_GPIO_DeInit(GPIO_Port[Led],GPIO_Pin[Led]);
}
/*
**************************************************************************************************
*����������LED
*������Led ��Ҫ������LED
*		 ��ѡ�Ĳ�����LED1��LED2��LED3��LED4
*���أ���
* ************************************************************************************************
 */
void BSP_LED_On(Led_TypeDef Led)
{
	HAL_GPIO_WritePin(GPIO_Port[Led],GPIO_Pin[Led],GPIO_PIN_RESET);
}
/*
**************************************************************************************************
*�������ر�LED
*������Led ��Ҫ�رյ�LED
*		 ��ѡ�Ĳ�����LED1��LED2��LED3��LED4
*���أ���
* ************************************************************************************************
 */
void BSP_LED_Off(Led_TypeDef Led)
{
	HAL_GPIO_WritePin(GPIO_Port[Led],GPIO_Pin[Led],GPIO_PIN_SET);
}
/*
**************************************************************************************************
*��������˸LED
*������Led ��Ҫ��˸��LED
*		 ��ѡ�Ĳ�����LED1��LED2��LED3��LED4
*���أ���
* ************************************************************************************************
 */
void BSP_LED_Toggle(Led_TypeDef Led)
{
	HAL_GPIO_TogglePin(GPIO_Port[Led],GPIO_Pin[Led]);
}
/*
**************************************************************************************************
*������������ʼ��
*������Button 		Ĭ�ϲ���User_Button
*	   Button_Mode	��ͨIOģʽ�������ж�ģʽ
*		 		BUTTON_MODE_GPIO  ��ͨIOģʽ
*		 		BUTTON_MODE_EXIT	�ж�ģʽ		  
*���أ���
* ************************************************************************************************
 */
void BSP_Button_Init(Button_TypeDef Button,ButtonMode_TypeDef Button_Mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*Enable the Button Clock*/
	BUTTON_GPIO_CLK_ENABLE();
	if (Button_Mode == Button_Mode_GPIO)
	{
		/*����ButtonΪ����ģʽ*/
		GPIO_InitStructure.Pin   = Button_Pin[Button];
		GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull  = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

		HAL_GPIO_Init(Button_Port[Button],&GPIO_InitStructure);
	}
	if (Button_Mode == Button_Mode_EXIT)
	{
		/*����ButtonΪ�ⲿ�ж�����ģʽ*/
		GPIO_InitStructure.Pin   = Button;
		GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull  = GPIO_NOPULL;
		GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

		HAL_GPIO_Init(Button_Port[Button],&GPIO_InitStructure);
		/*ʹ���ⲿ�жϣ�����Ϊ������ȼ�*/
		HAL_NVIC_SetPriority((IRQn_Type)(Button_IRQn[Button]),0x0f,0x00);
		HAL_NVIC_EnableIRQ((IRQn_Type)(Button_IRQn[Button]));
	}
}
/*
**************************************************************************************************
*������������ʼ��
*������Button 	Ĭ��User_Button	  	  
*���أ���
* ************************************************************************************************
 */
void BSP_Button_DeInit(Button_TypeDef Button)
{
	HAL_NVIC_DisableIRQ((IRQn_Type)(Button_IRQn[Button]));
   HAL_GPIO_DeInit(Button_Port[Button], Button_Pin[Button]);
}
/*
**************************************************************************************************
*��������ȡ������ֵ
*������Button 	Ĭ��User_Button	  
*���أ�������ֵ
* ************************************************************************************************
 */
uint32_t BSP_Button_GetState(Button_TypeDef Button)
{
  return HAL_GPIO_ReadPin(Button_Port[Button], Button_Pin[Button]);
}
/*
**************************************************************************************************
*������OTM8009A��ʱ����
*������delay ��ʱ����ֵ����λms	  
*���أ���
* ************************************************************************************************
 */
void OTM8009A_IO_Delay(uint32_t delay)
{
  HAL_Delay(delay);
}
/*
**************************************************************************************************
*������I2C1��ʼ������
*��������	  
*���أ���
* ************************************************************************************************
 */
static void I2C1_Init(void)
{
	if(HAL_I2C_GetState(&I2C1_Handle) == HAL_I2C_STATE_RESET)
	{
		I2C1_Handle.Instance = I2C1;
		/*����I2C1 Handle*/
		I2C1_Handle.Init.ClockSpeed      = I2C1_SCL_FREQ_KHZ;
		/*����ģʽ�� Tlow/Thigh = 2*/ 
		I2C1_Handle.Init.DutyCycle       = I2C_DUTYCYCLE_2;
		I2C1_Handle.Init.OwnAddress1     = 0;
		I2C1_Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
		I2C1_Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		I2C1_Handle.Init.OwnAddress2     = 0;
		/*��ֹ�㲥����ģʽ*/
		I2C1_Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		/*��ֹʱ���ӳ�ģʽ,ʱ���ӳ����忴�ֲ�*/
		I2C1_Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

		I2C1_MspInit();
		HAL_I2C_Init(&I2C1_Handle);
	}

}
/*
**************************************************************************************************
*������I2C2�ָ�Ĭ��
*��������	  
*���أ���
* ************************************************************************************************
 */
static void I2C1_DeInit(void)
{
	if(HAL_I2C_GetState(&I2C1_Handle) == HAL_I2C_STATE_READY)
	{
		/*��λI2C2������*/
		HAL_I2C_DeInit(&I2C1_Handle);
		I2C1_DeMspInit();
	}	
}
/*
**************************************************************************************************
*������I2C2��ʼ������
*��������	  
*���أ���
* ************************************************************************************************
 */
static void I2C2_Init(void)
{
		if(HAL_I2C_GetState(&I2C2_Handle) == HAL_I2C_STATE_RESET)
	{
		I2C2_Handle.Instance = I2C2;
		/*����I2C1 Handle*/
		I2C2_Handle.Init.ClockSpeed      = I2C2_SCL_FREQ_KHZ;
		/*����ģʽ�� Tlow/Thigh = 2*/ 
		I2C2_Handle.Init.DutyCycle       = I2C_DUTYCYCLE_2;
		I2C2_Handle.Init.OwnAddress1     = 0;
		I2C2_Handle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
		I2C2_Handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		I2C2_Handle.Init.OwnAddress2     = 0;
		/*��ֹ�㲥����ģʽ*/
		I2C2_Handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		/*��ֹʱ���ӳ�ģʽ,ʱ���ӳ����忴�ֲ�*/
		I2C2_Handle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

		I2C1_MspInit();
		HAL_I2C_Init(&I2C2_Handle);
	}
}
/*
**************************************************************************************************
*������I2C2�ָ�Ĭ�Ϻ���
*��������	  
*���أ���
* ************************************************************************************************
 */
static void I2C2_DeInit(void)
{
	if(HAL_I2C_GetState(&I2C2_Handle) == HAL_I2C_STATE_READY)
	{
		/*��λI2C2������*/
		HAL_I2C_DeInit(&I2C2_Handle);
		I2C1_DeMspInit();
	}	
}
/*
**************************************************************************************************
*������I2C1 MSP�����ʼ������
*��������	  
*���أ���
* ************************************************************************************************
 */
static void I2C1_MspInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*ʹ��GPIO��ʱ��*/
	BSP_I2C1_SCL_SDA_GPIO_CLK_ENABLE();
	/*����I2C1��ʱ���ź�*/
	GPIO_InitStructure.Pin = BSP_I2C1_SCL_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = BSP_I2C1_SCL_SDA_AF;
	HAL_GPIO_Init(BSP_I2C1_SCL_SDA_GPIO_PORT, &GPIO_InitStructure);

	/*����I2C1�������ź�*/
	GPIO_InitStructure.Pin = BSP_I2C1_SDA_PIN;
	HAL_GPIO_Init(BSP_I2C1_SCL_SDA_GPIO_PORT, &GPIO_InitStructure);
	/*ʹ��I2C1��ʱ��*/
	BSP_I2C1_CLK_ENABLE();
	/*ǿ�Ƹ�λI2C1��ʱ�Ӻ�����*/
	BSP_I2C1_FORCE_RESET();
	/*����I2C1��ʱ�Ӻ�����*/
	BSP_I2C1_RELEASE_RESET();

	/*����I2C1�¼��ж�*/
	HAL_NVIC_SetPriority(BSP_I2C1_EV_IRQn, 5 ,0);
	HAL_NVIC_EnableIRQ(BSP_I2C1_EV_IRQn);
	/*����I2C1�����ж�*/
	HAL_NVIC_SetPriority(BSP_I2C1_ER_IRQn, 5 ,0);
	HAL_NVIC_EnableIRQ(BSP_I2C1_ER_IRQn);
}
/*
**************************************************************************************************
*������I2C2 MSP����ָ�Ĭ��
*��������	  
*���أ���
* ************************************************************************************************
 */
static void I2C1_DeMspInit(void)
{
	/*��ֹI2C1���ж�*/
	HAL_NVIC_DisableIRQ(BSP_I2C1_EV_IRQn);
	HAL_NVIC_DisableIRQ(BSP_I2C1_ER_IRQn);
	/*ǿ�Ƹ�λI2C1��ʱ�Ӻ�����*/
	BSP_I2C1_FORCE_RESET();
	/*��ֹI2C1��ʱ��*/
	BSP_I2C1_CLK_DISABLE();
}
/*
**************************************************************************************************
*������I2C2 MSP�����ʼ������
*��������	  
*���أ���
* ************************************************************************************************
 */
static void I2C2_MspInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*ʹ��GPIO��ʱ��*/
	BSP_I2C1_SCL_SDA_GPIO_CLK_ENABLE();
	/*����I2C1��ʱ���ź�*/
	GPIO_InitStructure.Pin = BSP_I2C2_SCL_PIN;
	GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = BSP_I2C2_SCL_SDA_AF;
	HAL_GPIO_Init(BSP_I2C2_SCL_SDA_GPIO_PORT, &GPIO_InitStructure);

	/*����I2C1�������ź�*/
	GPIO_InitStructure.Pin = BSP_I2C2_SDA_PIN;
	HAL_GPIO_Init(BSP_I2C2_SCL_SDA_GPIO_PORT, &GPIO_InitStructure);
	/*ʹ��I2C1��ʱ��*/
	BSP_I2C1_CLK_ENABLE();
	/*ǿ�Ƹ�λI2C1��ʱ�Ӻ�����*/
	BSP_I2C1_FORCE_RESET();
	/*����I2C1��ʱ�Ӻ�����*/
	BSP_I2C1_RELEASE_RESET();

	/*����I2C1�¼��ж�*/
	HAL_NVIC_SetPriority(BSP_I2C2_EV_IRQn, 5 ,0);
	HAL_NVIC_EnableIRQ(BSP_I2C2_EV_IRQn);
	/*����I2C1�����ж�*/
	HAL_NVIC_SetPriority(BSP_I2C2_ER_IRQn, 5 ,0);
	HAL_NVIC_EnableIRQ(BSP_I2C2_ER_IRQn);
}
/*
**************************************************************************************************
*������I2C2 MSP����ָ�Ĭ�Ϻ���
*��������	  
*���أ���
* ************************************************************************************************
 */
static void I2C2_DeMspInit(void)
{
	/*��ֹI2C1���ж�*/
	HAL_NVIC_DisableIRQ(BSP_I2C2_EV_IRQn);
	HAL_NVIC_DisableIRQ(BSP_I2C2_ER_IRQn);
	/*ǿ�Ƹ�λI2C1��ʱ�Ӻ�����*/
	BSP_I2C2_FORCE_RESET();
	/*��ֹI2C1��ʱ��*/
	BSP_I2C2_CLK_DISABLE();	
}
/*
**************************************************************************************************
*������I2C1	д�������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ�����ַ
*		 Valueд�����ݵ�ֵ	  
*���أ����ز������
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
*������I2C1	���������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ�����ַ	  
*���أ����ض�������
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
*������I2C1	��������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ�����ַ	
*		 MemAddsize  ���ݵĴ�С
*		 Buffer ���ݻ���ĵ�ַ
*		 Length ���ݵĳ���  
*���أ����ز������
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
*������I2C2	��������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ�����ַ	
*		 MemAddsize  ���ݵĴ�С
*		 Buffer ���ݻ���ĵ�ַ
*		 Length ���ݵĳ���  
*���أ����ز������
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
*������I2C1	д������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ�����ַ	
*		 MemAddsize  ���ݵĴ�С
*		 Buffer ���ݻ���ĵ�ַ
*		 Length ���ݵĳ���  
*���أ����ز������
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
*������I2C2	д������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ�����ַ	
*		 MemAddsize  ���ݵĴ�С
*		 Buffer ���ݻ���ĵ�ַ
*		 Length ���ݵĳ���  
*���أ����ز������
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
*������I2C1	д������ݺ���
*������DevAddress I2C�豸��ַ
*		 Trials   ���ԵĴ��� 
*���أ����ز������
* ************************************************************************************************
 */
static HAL_StatusTypeDef I2C1_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{
	return (HAL_I2C_IsDeviceReady(&I2C2_Handle, DevAddress, Trials, 1000));
}
/*
**************************************************************************************************
*������I2C2	д������ݺ���
*������DevAddress I2C�豸��ַ
*		 Trials   ���ԵĴ��� 
*���أ����ز������
* ************************************************************************************************
 */
static HAL_StatusTypeDef I2C2_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{
	return (HAL_I2C_IsDeviceReady(&I2C2_Handle, DevAddress, Trials, 1000));
}
/*
**************************************************************************************************
*������I2C1	��������
*������Addr I2C�豸��ַ
*���أ���
* ************************************************************************************************
 */
static void I2C1_Error(uint8_t Addr)
{
	HAL_I2C_DeInit(&I2C1_Handle);
	I2C1_Init();
}
/*
**************************************************************************************************
*������I2C1	��������
*������Addr I2C�豸��ַ
*���أ���
* ************************************************************************************************
 */
static void I2C2_Error(uint8_t Addr)
{
	HAL_I2C_DeInit(&I2C2_Handle);
	I2C2_Init();
}
/*
**********************************************************************************************************
*												TS�ӿں���
* ********************************************************************************************************
 */
/*
**************************************************************************************************
*����������IO��ʼ������
*��������
*���أ���
* ************************************************************************************************
 */
void TS_IO_Init(void)
{
	I2C1_Init();
}
/*
**************************************************************************************************
*����������IO��λ����
*��������
*���أ���
* ************************************************************************************************
 */
void TS_IO_DeInit(void)
{
	I2C1_DeInit();
}
/*
**************************************************************************************************
*����������IOд�������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ��ַ
*		 Value ���ݵ�ֵ
*���أ���
* ************************************************************************************************
 */
uint8_t TS_IO_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
	return I2C1_Write(Addr, Reg, Value);
}
/*
**************************************************************************************************
*����������IO���������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ��ַ
*���أ����ض�ȡ����ֵ
* ************************************************************************************************
 */
uint8_t TS_IO_Read(uint8_t Addr, uint8_t Reg)
{
	return I2C1_Read(Addr, Reg);
}
/*
**************************************************************************************************
*����������IO��������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ��ַ
*		 MemAddsize ���ݵĴ�С
*		 Buffer  ���ݻ����ַ
*		 Length  ���ݳ���
*���أ���
* ************************************************************************************************
 */
uint8_t TS_IO_ReadMultiple(uint8_t Addr, uint16_t Reg, uint8_t *Buffer, uint16_t Length)
{
	return I2C1_ReadMultiple(Addr, Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
}
/*
**************************************************************************************************
*����������IOд������ݺ���
*������Addr I2C�豸��ַ
*		 Reg  �Ĵ��ַ
*		 MemAddsize ���ݵĴ�С
*		 Buffer  ���ݻ����ַ
*		 Length  ���ݳ���
*���أ���
* ************************************************************************************************
 */
uint8_t TS_IO_WriteMultiple(uint8_t Addr, uint16_t Reg, uint8_t *Buffer, uint16_t Length)
{
	 return I2C1_WriteMultiple(Addr, Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
}
/*
**************************************************************************************************
*����������IO��ʱ����
*������Delay ��ʱ������λms
*���أ���
* ************************************************************************************************
 */
void TS_IO_Delay(uint32_t Delay)
{
	HAL_Delay(Delay);
}
/*
**********************************************************************************************************
*												Audio�ӿں���
* ********************************************************************************************************
 */

/*
**********************************************************************************************************
*												MFRC522-SPI�ӿں���
* ********************************************************************************************************
 */
