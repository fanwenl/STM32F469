/**
  ******************************************************************************
  * @file    Templates/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.3
  * @date    06-May-2016
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
//void HardFault_Handler(void)
//{
//  /* Go to infinite loop when Hard Fault exception occurs */
//
//	__asm("TST LR, #4");
//	__asm("ITE EQ");
//	__asm("MRSEQ R0, MSP");
//	__asm("MRSNE R0, PSP");
//	__asm("MOV R1, LR");
//	__asm("B HardFault_Handler_C");
//
//  while (1)
//  {
//  }
//}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	OS_CPU_SR  cpu_sr;
  
	HAL_IncTick();
  if(OSRunning > 0u)						/*系统运行之后在开始调度*/					
  {
		OS_ENTER_CRITICAL();          /* Tell uC/OS-II that we are starting an ISR            */
		OSIntEnter();
		OS_EXIT_CRITICAL();
		
		OSTimeTick();                 /* Call uC/OS-II's OSTimeTick()                         */
		OSIntExit();
  }
}
void HardFault_Handler_C(unsigned long * hardfault_args, unsigned int lr_value)
{
	unsigned long stacked_r0;
	unsigned long stacked_r1;
	unsigned long stacked_r2;
	unsigned long stacked_r3;
	unsigned long stacked_r12;
	unsigned long stacked_lr;
	unsigned long stacked_pc;
	unsigned long stacked_psr;
	unsigned long cfsr;
	unsigned long bus_fault_address;
	unsigned long memmanage_fault_address;
	bus_fault_address = SCB->BFAR;
	memmanage_fault_address = SCB->MMFAR;
	cfsr = SCB->CFSR;
	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);
	stacked_r12 = ((unsigned long) hardfault_args[4]);
	stacked_lr = ((unsigned long) hardfault_args[5]);
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);
	printf ("[HardFault]\n");
	printf ("- Stack frame:\n");
	printf (" R0 = %x\n", stacked_r0);
	printf (" R1 = %x\n", stacked_r1);
	printf (" R2 = %x\n", stacked_r2);
	printf (" R3 = %x\n", stacked_r3);
	printf (" R12 = %x\n", stacked_r12);
	printf (" LR = %x\n", stacked_lr);
	printf (" PC = %x\n", stacked_pc);
	printf (" PSR = %x\n", stacked_psr);
	printf ("- FSR/FAR:\n");
	printf (" CFSR = %x\n", cfsr);
	printf (" HFSR = %x\n", SCB->HFSR);
	printf (" DFSR = %x\n", SCB->DFSR);
	printf (" AFSR = %x\n", SCB->AFSR);
	if (cfsr & 0x0080) printf (" MMFAR = %x\n",memmanage_fault_address);
	if (cfsr & 0x8000) printf (" BFAR = %x\n", bus_fault_address);
	printf ("- Misc\n");
	printf (" LR/EXC_RETURN= %x\n", lr_value);
	while(1); // endless loop
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
