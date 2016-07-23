#include "app_hooks.h"


uint16_t time = 0u;   /*系统时钟节拍全局变量*/

/**
 * [App_TimeTickHook 系统时钟节拍Hook函数]
 */
void App_TimeTickHook(void)
{
	if(time == 1000u)
	{
		time = 0;
		BSP_LED_Toggle(LED4); /*系统时钟指示灯*/
	}
	else
	{
		time++;
	}
}

void App_TaskSwHook(void)
{

}
void App_TaskCreateHook(OS_TCB *ptcb)
{}
void App_TaskDelHook(OS_TCB *ptcb)
{}
void App_TCBInitHook(OS_TCB *ptcb)
{}
void App_TaskIdleHook(void)
{
	BSP_LED_Toggle(LED3);	
}
void App_TaskReturnHook(OS_TCB *ptcb)
{}
void App_TaskStatHook(void)
{}

