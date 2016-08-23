#ifndef __APP_CFG_H__
#define __APP_CFG_H__

/*GUI_X_uCOS使用*/
#define FALSE   0u   
#define TRUE	 1u

#define OS_TASK_TMR_PRIO   (OS_LOWEST_PRIO - 2u)
/*********************�����������ȼ�***********************/
#define APP_CFG_START_TASK_PRIO    2
#define APP_CFG_MAIN_TASK_PRIO    3
#define APP_CFG_MONITOR_TASK_PRIO    4

/*********************���������ջ��С*********************/
#define APP_CFG_START_TASK_STK_SIZE         256
#define APP_CFG_MAIN_TASK_STK_SIZE         512
#define APP_CFG_MONITOR_TASK_STK_SIZE       256

#endif