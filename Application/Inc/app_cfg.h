#ifndef __APP_CFG_H__
#define __APP_CFG_H__

#define OS_TASK_TMR_PRIO   (OS_LOWEST_PRIO - 2u)
/*********************�����������ȼ�***********************/
#define APP_CFG_TASK_START_PRIO    2
#define APP_CFG_MAIN_TASK_PRIO    3
#define APP_CFG_MONITOR_TASK_PRIO    4
#define APP2_CFG_TASK_PRIO    6
#define APP3_CFG_TASK_PRIO    7
/*********************���������ջ��С*********************/
#define APP_CFG_TASK_START_STK_SIZE         256
#define APP_CFG_MAIN_TASK_STK_SIZE         256
#define APP_CFG_MONITOR_TASK_STK_SIZE        128
#define APP2_CFG_TASK_START_STK_SIZE        128
#define APP3_CFG_TASK_START_STK_SIZE        128
#endif