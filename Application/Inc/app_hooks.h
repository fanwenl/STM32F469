#ifndef __APP_HOOKS_H
#define __APP_HOOKS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"
#include "ucos_ii.h"

void App_TimeTickHook(void);
void App_TaskSwHook(void);
void App_TaskCreateHook(OS_TCB *ptcb);
void App_TaskDelHook(OS_TCB *ptcb);
void App_TaskIdleHook(void);
void App_TaskReturnHook(OS_TCB *ptcb);
void App_TaskStatHook(void);
void App_TCBInitHook(OS_TCB *ptcb);

#ifdef __cplusplus
}
#endif /*end cplusplus*/

#endif /*__APP_HOOKS_H*/

