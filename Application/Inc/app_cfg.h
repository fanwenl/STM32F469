#ifndef __APP_CFG_H__
#define __APP_CFG_H__

/*GUI_X_uCOS浣跨敤*/
#define FALSE   0u   
#define TRUE	 1u

#define OS_TASK_TMR_PRIO   (OS_LOWEST_PRIO - 2u)
/*********************定义任务优先级***********************/
#define APP_CFG_START_TASK_PRIO    2
#define APP_CFG_MAIN_TASK_PRIO    3
#define APP_CFG_MONITOR_TASK_PRIO    4

/*********************定义任务堆栈大小*********************/
#define APP_CFG_START_TASK_STK_SIZE         256
#define APP_CFG_MAIN_TASK_STK_SIZE         512
#define APP_CFG_MONITOR_TASK_STK_SIZE       256

#endif