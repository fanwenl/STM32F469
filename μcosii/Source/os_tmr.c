/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*                                            TIMER MANAGEMENT
*
*                              (c) Copyright 1992-2013, Micrium, Weston, FL
*                                           All Rights Reserved
*
*
* File    : OS_TMR.C
* By      : Jean J. Labrosse
* Version : V2.92.11
*
* LICENSING TERMS:
* ---------------
*   uC/OS-II is provided in source form for FREE evaluation, for educational use or for peaceful research.
* If you plan on using  uC/OS-II  in a commercial product you need to contact Micrium to properly license
* its use in your product. We provide ALL the source code for your convenience and to help you experience
* uC/OS-II.   The fact that the  source is provided does  NOT  mean that you can use it without  paying a
* licensing fee.
*********************************************************************************************************
*/

#define  MICRIUM_SOURCE

#ifndef  OS_MASTER_FILE
#include "ucos_ii.h"
#endif

/*
*********************************************************************************************************
*                                                        NOTES
*
* 1) Your application MUST define the following #define constants:
*
*    OS_TASK_TMR_PRIO          The priority of the Timer management task
*    OS_TASK_TMR_STK_SIZE      The size     of the Timer management task's stack
*
* 2) You must call OSTmrSignal() to notify the Timer management task that it's time to update the timers.
* 3) OSTmrMatch和OSTmrTime两个变量在OSTmr_Init()中初始化为0，值溢出后从0开始计数。
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define  OS_TMR_LINK_DLY       0u
#define  OS_TMR_LINK_PERIODIC  1u

/*
*********************************************************************************************************
*                                          LOCAL PROTOTYPES
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  OS_TMR  *OSTmr_Alloc         (void);
static  void     OSTmr_Free          (OS_TMR *ptmr);
static  void     OSTmr_InitTask      (void);
static  void     OSTmr_Link          (OS_TMR *ptmr, INT8U type);
static  void     OSTmr_Unlink        (OS_TMR *ptmr);
static  void     OSTmr_Task          (void   *p_arg);
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           CREATE A TIMER
*
* Description: This function is called by your application code to create a timer.
*
* Arguments  : dly           Initial delay.
*                            If the timer is configured for ONE-SHOT mode, this is the timeout used.
*                            If the timer is configured for PERIODIC mode, this is the first timeout to 
*                               wait for before the timer starts entering periodic mode.
*                               (再次启动定时器时的一段延时时间)
*              period        The 'period' being repeated for the timer.
*                               If you specified 'OS_TMR_OPT_PERIODIC' as an option, when the timer 
*                               expires, it will automatically restart with the same period.
*
*              opt           Specifies either:
*                               OS_TMR_OPT_ONE_SHOT       The timer counts down only once
*                               OS_TMR_OPT_PERIODIC       The timer counts down and then reloads itself
*
*              callback      Is a pointer to a callback function that will be called when the timer expires. 
*                               The callback function must be declared as follows:
*
*                               void MyCallback (OS_TMR *ptmr, void *p_arg);
*
*              callback_arg  Is an argument (a pointer) that is passed to the callback function when it is called.
*
*              pname         Is a pointer to an ASCII string that is used to name the timer.  Names are 
*                               useful for debugging.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID_DLY     you specified an invalid delay
*                               OS_ERR_TMR_INVALID_PERIOD  you specified an invalid period
*                               OS_ERR_TMR_INVALID_OPT     you specified an invalid option
*                               OS_ERR_TMR_ISR             if the call was made from an ISR
*                               OS_ERR_TMR_NON_AVAIL       if there are no free timers from the timer pool
*
* Returns    : A pointer to an OS_TMR data structure.
*              This is the 'handle' that your application will use to reference the timer created.
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
OS_TMR  *OSTmrCreate (INT32U           dly,
                      INT32U           period,
                      INT8U            opt,
                      OS_TMR_CALLBACK  callback,
                      void            *callback_arg,
                      INT8U           *pname,
                      INT8U           *perr)
{
    OS_TMR   *ptmr;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_TMR *)0);
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((OS_TMR *)0);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    switch (opt) {                                          /* Validate arguments                                     */
        case OS_TMR_OPT_PERIODIC:
             if (period == 0u) {
                 *perr = OS_ERR_TMR_INVALID_PERIOD;
                 return ((OS_TMR *)0);
             }
             break;

        case OS_TMR_OPT_ONE_SHOT:
             if (dly == 0u) {
                 *perr = OS_ERR_TMR_INVALID_DLY;
                 return ((OS_TMR *)0);
             }
             break;

        default:
             *perr = OS_ERR_TMR_INVALID_OPT;
             return ((OS_TMR *)0);
    }
#endif
    if (OSIntNesting > 0u) {                                /* See if trying to call from an ISR                      */
        *perr  = OS_ERR_TMR_ISR;
        return ((OS_TMR *)0);
    }
    OSSchedLock();                                          /*锁定调度器*/
    ptmr = OSTmr_Alloc();                                   /* Obtain a timer from the free pool                      */
    if (ptmr == (OS_TMR *)0) {
        OSSchedUnlock();
        *perr = OS_ERR_TMR_NON_AVAIL;
        return ((OS_TMR *)0);
    }
    ptmr->OSTmrState       = OS_TMR_STATE_STOPPED;          /* Indicate that timer is not running yet                 */
    ptmr->OSTmrDly         = dly;                           /*参数赋值*/
    ptmr->OSTmrPeriod      = period;
    ptmr->OSTmrOpt         = opt;
    ptmr->OSTmrCallback    = callback;
    ptmr->OSTmrCallbackArg = callback_arg;
#if OS_TMR_CFG_NAME_EN > 0u
    if (pname == (INT8U *)0) {                              /* Is 'pname' a NULL pointer?                             */
        ptmr->OSTmrName    = (INT8U *)(void *)"?";
    } else {
        ptmr->OSTmrName    = pname;
    }
#endif
    OSSchedUnlock();
    *perr = OS_ERR_NONE;
    return (ptmr);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           DELETE A TIMER
*
* Description: This function is called by your application code to delete a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to stop and delete.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID        'ptmr'  is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR            if the function was called from an ISR
*                               OS_ERR_TMR_INACTIVE       if the timer was not created
*                               OS_ERR_TMR_INVALID_STATE  the timer is in an invalid state
*
* Returns    : OS_TRUE       If the call was successful
*              OS_FALSE      If not
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
BOOLEAN  OSTmrDel (OS_TMR  *ptmr,
                   INT8U   *perr)
{
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (OS_FALSE);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (OS_FALSE);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {                   /* Validate timer structure                               */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (OS_FALSE);
    }
    if (OSIntNesting > 0u) {                                /* See if trying to call from an ISR                      */
        *perr  = OS_ERR_TMR_ISR;
        return (OS_FALSE);
    }
    OSSchedLock();
    switch (ptmr->OSTmrState) {                            /*判断定时器的状态，根据不同的状态删除timer*/
        case OS_TMR_STATE_RUNNING:
             OSTmr_Unlink(ptmr);                            /* Remove from current wheel spoke                        */
             OSTmr_Free(ptmr);                              /* Return timer to free list of timers                    */
             OSSchedUnlock();
             *perr = OS_ERR_NONE;
             return (OS_TRUE);

        case OS_TMR_STATE_STOPPED:                          /* Timer has not started or ...                           */
        case OS_TMR_STATE_COMPLETED:                        /* ... timer has completed the ONE-SHOT time              */
             OSTmr_Free(ptmr);                              /* Return timer to free list of timers                    */
             OSSchedUnlock();
             *perr = OS_ERR_NONE;
             return (OS_TRUE);

        case OS_TMR_STATE_UNUSED:                           /* Already deleted                                        */
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INACTIVE;
             return (OS_FALSE);

        default:
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INVALID_STATE;
             return (OS_FALSE);
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                       GET THE NAME OF A TIMER
*
* Description: This function is called to obtain the name of a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to obtain the name for
*
*              pdest         Is a pointer to pointer to where the name of the timer will be placed.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE               The call was successful
*                               OS_ERR_TMR_INVALID_DEST   'pdest' is a NULL pointer
*                               OS_ERR_TMR_INVALID        'ptmr'  is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_NAME_GET_ISR       if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE       'ptmr'  points to a timer that is not active
*                               OS_ERR_TMR_INVALID_STATE  the timer is in an invalid state
*
* Returns    : The length of the string or 0 if the timer does not exist.
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u && OS_TMR_CFG_NAME_EN > 0u
INT8U  OSTmrNameGet (OS_TMR   *ptmr,
                     INT8U   **pdest,
                     INT8U    *perr)
{
    INT8U  len;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (pdest == (INT8U **)0) {
        *perr = OS_ERR_TMR_INVALID_DEST;
        return (0u);
    }
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (0u);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (0u);
    }
    if (OSIntNesting > 0u) {                           /* See if trying to call from an ISR                           */
        *perr = OS_ERR_NAME_GET_ISR;
        return (0u);
    }
    OSSchedLock();                                    /*如果该timer使用过，则有name，否则没有name*/
    switch (ptmr->OSTmrState) {
        case OS_TMR_STATE_RUNNING:
        case OS_TMR_STATE_STOPPED:
        case OS_TMR_STATE_COMPLETED:
             *pdest = ptmr->OSTmrName;
             len    = OS_StrLen(*pdest);
             OSSchedUnlock();
             *perr = OS_ERR_NONE;
             return (len);

        case OS_TMR_STATE_UNUSED:                      /* Timer is not allocated                                      */
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INACTIVE;
             return (0u);

        default:
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INVALID_STATE;
             return (0u);
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                          GET HOW MUCH TIME IS LEFT BEFORE A TIMER EXPIRES
*
* Description: This function is called to get the number of ticks before a timer times out.
*              (定时时间到之前，剩余的ticks，如果要算剩余的时间：ticks乘以定时器的最小精度)
* Arguments  : ptmr          Is a pointer to the timer to obtain the remaining time from.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID        'ptmr' is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR            if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE       'ptmr' points to a timer that is not active
*                               OS_ERR_TMR_INVALID_STATE  the timer is in an invalid state
*
* Returns    : The time remaining for the timer to expire.  The time represents 'timer' increments(增加). 
*              In other words, if OSTmr_Task() is signaled every 1/10 of a second then the returned 
*              value represents the number of 1/10 of a second remaining before the timer expires.
*********************************************************************************************************
*/
#if OS_TMR_EN > 0u
INT32U  OSTmrRemainGet (OS_TMR  *ptmr,
                        INT8U   *perr)
{
    INT32U  remain;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (0u);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (0u);
    }
    if (OSIntNesting > 0u) {                           /* See if trying to call from an ISR                           */
        *perr = OS_ERR_TMR_ISR;
        return (0u);
    }
    OSSchedLock();
    switch (ptmr->OSTmrState) {                         /*区分不同的情况*/
        case OS_TMR_STATE_RUNNING:
             remain = ptmr->OSTmrMatch - OSTmrTime;    /* Determine how much time is left to timeout                  */
             OSSchedUnlock();
             *perr  = OS_ERR_NONE;
             return (remain);

        case OS_TMR_STATE_STOPPED:                     /* It's assumed that the timer has not started yet             */
             switch (ptmr->OSTmrOpt) {
                 case OS_TMR_OPT_PERIODIC:
                      if (ptmr->OSTmrDly == 0u) {
                          remain = ptmr->OSTmrPeriod;    /*Dly为0返回period*/
                      } else {
                          remain = ptmr->OSTmrDly;      /*否则放回dly*/
                      }
                      OSSchedUnlock();
                      *perr  = OS_ERR_NONE;
                      break;

                 case OS_TMR_OPT_ONE_SHOT:               /*单次延时返回Dly*/
                 default:
                      remain = ptmr->OSTmrDly;
                      OSSchedUnlock();
                      *perr  = OS_ERR_NONE;
                      break;
             }
             return (remain);
                                                        /*注意区分stop和completed的区别看英文注释*/
        case OS_TMR_STATE_COMPLETED:                   /* Only ONE-SHOT that timed out can be in this state           */
             OSSchedUnlock();
             *perr = OS_ERR_NONE;
             return (0u);                               /*已经完成放回0*/

        case OS_TMR_STATE_UNUSED:                       /*定时器没有使用*/
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INACTIVE;
             return (0u);

        default:
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INVALID_STATE;
             return (0u);
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                  FIND OUT WHAT STATE A TIMER IS IN
*
* Description: This function is called to determine what state the timer is in:
*
*                  OS_TMR_STATE_UNUSED     the timer has not been created
*                  OS_TMR_STATE_STOPPED    the timer has been created but has not been started or has been stopped
*                  OS_TMR_STATE_COMPLETED  the timer is in ONE-SHOT mode and has completed it's timeout
*                  OS_TMR_STATE_RUNNING    the timer is currently running
*
* Arguments  : ptmr          Is a pointer to the desired timer
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID        'ptmr' is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR            if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE       'ptmr' points to a timer that is not active
*                               OS_ERR_TMR_INVALID_STATE  if the timer is not in a valid state
*
* Returns    : The current state of the timer (see description).
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
INT8U  OSTmrStateGet (OS_TMR  *ptmr,
                      INT8U   *perr)
{
    INT8U  state;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (0u);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (0u);
    }
    if (OSIntNesting > 0u) {                           /* See if trying to call from an ISR                           */
        *perr = OS_ERR_TMR_ISR;
        return (0u);
    }
    OSSchedLock();
    state = ptmr->OSTmrState;
    switch (state) {
        case OS_TMR_STATE_UNUSED:
        case OS_TMR_STATE_STOPPED:
        case OS_TMR_STATE_COMPLETED:
        case OS_TMR_STATE_RUNNING:
             *perr = OS_ERR_NONE;                     /*区分错误的类型*/
             break;

        default:
             *perr = OS_ERR_TMR_INVALID_STATE;
             break;
    }
    OSSchedUnlock();
    return (state);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                            START A TIMER
*
* Description: This function is called by your application code to start a timer.
*               (将定时器timer链接到time wheel中)
* Arguments  : ptmr          Is a pointer to an OS_TMR
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID
*                               OS_ERR_TMR_INVALID_TYPE    'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR             if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE        if the timer was not created
*                               OS_ERR_TMR_INVALID_STATE   the timer is in an invalid state
*
* Returns    : OS_TRUE    if the timer was started
*              OS_FALSE   if an error was detected
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
BOOLEAN  OSTmrStart (OS_TMR   *ptmr,
                     INT8U    *perr)
{
#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (OS_FALSE);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (OS_FALSE);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {                   /* Validate timer structure                               */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (OS_FALSE);
    }
    if (OSIntNesting > 0u) {                                /* See if trying to call from an ISR                      */
        *perr  = OS_ERR_TMR_ISR;
        return (OS_FALSE);
    }
    OSSchedLock();
    switch (ptmr->OSTmrState) {
        case OS_TMR_STATE_RUNNING:                          /* Restart the timer                                      */
             OSTmr_Unlink(ptmr);                            /* ... Stop the timer                                     */
             OSTmr_Link(ptmr, OS_TMR_LINK_DLY);             /* ... Link timer to timer wheel                          */
             OSSchedUnlock();                               /*重新link定时器*/
             *perr = OS_ERR_NONE;
             return (OS_TRUE);

        case OS_TMR_STATE_STOPPED:                          /* Start the timer                                        */
        case OS_TMR_STATE_COMPLETED:
             OSTmr_Link(ptmr, OS_TMR_LINK_DLY);             /* ... Link timer to timer wheel                          */
             OSSchedUnlock();
             *perr = OS_ERR_NONE;
             return (OS_TRUE);

        case OS_TMR_STATE_UNUSED:                           /* Timer not created                                      */
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INACTIVE;
             return (OS_FALSE);

        default:
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INVALID_STATE;
             return (OS_FALSE);
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                            STOP A TIMER
*
* Description: This function is called by your application code to stop a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to stop.
*
*              opt           Allows you to specify an option to this functions which can be:
*
*                               OS_TMR_OPT_NONE          Do nothing special but stop the timer(不做任何事)
*                               OS_TMR_OPT_CALLBACK      Execute the callback function, pass it the 
*                                                        callback argument specified when the timer 
*                                                        was created(调用回调函数使用创建定时器时的参数).
*                               OS_TMR_OPT_CALLBACK_ARG  Execute the callback function, pass it the 
*                                                        callback argument specified in THIS function call.
*                                                         (调用回调函数使用该函数声明中的参数callback_arg)
*              callback_arg  Is a pointer to a 'new' callback argument that can be passed to the callback 
*                            function instead of the timer's callback argument.
*                             (一个新的callback参数，传递给回调函数)  
*                             In other words, use 'callback_arg' passed in THIS function INSTEAD of 
*                               ptmr->OSTmrCallbackArg.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID         'ptmr' is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE    'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR             if the function was called from an ISR
*                               OS_ERR_TMR_INACTIVE        if the timer was not created
*                               OS_ERR_TMR_INVALID_OPT     if you specified an invalid option for 'opt'
*                               OS_ERR_TMR_STOPPED         if the timer was already stopped
*                               OS_ERR_TMR_INVALID_STATE   the timer is in an invalid state
*                               OS_ERR_TMR_NO_CALLBACK     if the timer does not have a callback function defined
*
* Returns    : OS_TRUE       If we stopped the timer (if the timer is already stopped, we also return OS_TRUE)
*              OS_FALSE      If not
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
BOOLEAN  OSTmrStop (OS_TMR  *ptmr,
                    INT8U    opt,
                    void    *callback_arg,
                    INT8U   *perr)
{
    OS_TMR_CALLBACK  pfnct;


#ifdef OS_SAFETY_CRITICAL
    if (perr == (INT8U *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (OS_FALSE);
    }
#endif

#if OS_ARG_CHK_EN > 0u
    if (ptmr == (OS_TMR *)0) {
        *perr = OS_ERR_TMR_INVALID;
        return (OS_FALSE);
    }
#endif
    if (ptmr->OSTmrType != OS_TMR_TYPE) {                         /* Validate timer structure                         */
        *perr = OS_ERR_TMR_INVALID_TYPE;
        return (OS_FALSE);
    }
    if (OSIntNesting > 0u) {                                      /* See if trying to call from an ISR                */
        *perr  = OS_ERR_TMR_ISR;
        return (OS_FALSE);
    }
    OSSchedLock();
    switch (ptmr->OSTmrState) {
        case OS_TMR_STATE_RUNNING:
             OSTmr_Unlink(ptmr);                                  /* Remove from current wheel spoke                  */
             *perr = OS_ERR_NONE;
             switch (opt) {
                 case OS_TMR_OPT_CALLBACK:                        /*停止定时器的时候调用回调函数*/
                      pfnct = ptmr->OSTmrCallback;                /* Execute callback function if available ...       */
                      if (pfnct != (OS_TMR_CALLBACK)0) {
                          (*pfnct)((void *)ptmr, ptmr->OSTmrCallbackArg);  /* Use callback arg when timer was created */
                      } else {
                          *perr = OS_ERR_TMR_NO_CALLBACK;
                      }
                      break;

                 case OS_TMR_OPT_CALLBACK_ARG:
                      pfnct = ptmr->OSTmrCallback;                /* Execute callback function if available ...       */
                      if (pfnct != (OS_TMR_CALLBACK)0) {
                          (*pfnct)((void *)ptmr, callback_arg);   /* ... using the 'callback_arg' provided in call    */
                      } else {
                          *perr = OS_ERR_TMR_NO_CALLBACK;
                      }
                      break;

                 case OS_TMR_OPT_NONE:
                      break;

                 default:
                     *perr = OS_ERR_TMR_INVALID_OPT;
                     break;
             }
             OSSchedUnlock();
             return (OS_TRUE);

        case OS_TMR_STATE_COMPLETED:                              /* Timer has already completed the ONE-SHOT or ...  */
        case OS_TMR_STATE_STOPPED:                                /* ... timer has not started yet.                   */
             OSSchedUnlock();
             *perr = OS_ERR_TMR_STOPPED;
             return (OS_TRUE);

        case OS_TMR_STATE_UNUSED:                                 /* Timer was not created                            */
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INACTIVE;
             return (OS_FALSE);

        default:
             OSSchedUnlock();
             *perr = OS_ERR_TMR_INVALID_STATE;
             return (OS_FALSE);
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                             SIGNAL THAT IT'S TIME TO UPDATE THE TIMERS
*
* Description: This function is typically called by the ISR that occurs at the timer tick rate and is 
*              used to signal to OSTmr_Task() that it's time to update the timers.
*
* Arguments  : none
*
* Returns    : OS_ERR_NONE         The call was successful and the timer task was signaled.
*              OS_ERR_SEM_OVF      If OSTmrSignal() was called more often than OSTmr_Task() can handle 
*                                  the timers. This would indicate that your system is heavily loaded.
*              OS_ERR_EVENT_TYPE   Unlikely you would get this error because the semaphore used for 
*                                  signaling is created by uC/OS-II.
*              OS_ERR_PEVENT_NULL  Again, unlikely you would ever get this error because the semaphore 
*                                  used for signaling is created by uC/OS-II.
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
INT8U  OSTmrSignal (void)
{
    INT8U  err;


    err = OSSemPost(OSTmrSemSignal);   /*发送信号量*/
    return (err);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                      ALLOCATE AND FREE A TIMER
*
* Description: This function is called to allocate(分配) a timer.
*
* Arguments  : none
*
* Returns    : a pointer to a timer if one is available
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  OS_TMR  *OSTmr_Alloc (void)
{
    OS_TMR *ptmr;


    if (OSTmrFreeList == (OS_TMR *)0) {
        return ((OS_TMR *)0);
    }
    ptmr            = (OS_TMR *)OSTmrFreeList;
    OSTmrFreeList   = (OS_TMR *)ptmr->OSTmrNext;      /*从定时器列表中取出一个空的定时器*/
    ptmr->OSTmrNext = (OS_TCB *)0;
    ptmr->OSTmrPrev = (OS_TCB *)0;
    OSTmrUsed++;
    OSTmrFree--;
    return (ptmr);
}
#endif


/*
*********************************************************************************************************
*                                   RETURN A TIMER TO THE FREE LIST
*
* Description: This function is called to return a timer object to the free list of timers.
*
* Arguments  : ptmr     is a pointer to the timer to free
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  void  OSTmr_Free (OS_TMR *ptmr)
{
    ptmr->OSTmrState       = OS_TMR_STATE_UNUSED;      /* Clear timer object fields(清除timer的参数)                  */
    ptmr->OSTmrOpt         = OS_TMR_OPT_NONE;
    ptmr->OSTmrPeriod      = 0u;
    ptmr->OSTmrMatch       = 0u;
    ptmr->OSTmrCallback    = (OS_TMR_CALLBACK)0;
    ptmr->OSTmrCallbackArg = (void *)0;
#if OS_TMR_CFG_NAME_EN > 0u
    ptmr->OSTmrName        = (INT8U *)(void *)"?";
#endif

    ptmr->OSTmrPrev        = (OS_TCB *)0;              /* Chain timer to free list                                    */
    ptmr->OSTmrNext        = OSTmrFreeList;
    OSTmrFreeList          = ptmr;

    OSTmrUsed--;                                       /* Update timer object statistics                              */
    OSTmrFree++;
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                                    INITIALIZATION
*                                          INITIALIZE THE FREE LIST OF TIMERS
*
* Description: This function is called by OSInit() to initialize the free list of OS_TMRs.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
void  OSTmr_Init (void)
{
#if OS_EVENT_NAME_EN > 0u
    INT8U    err;
#endif
    INT16U   ix;
    INT16U   ix_next;
    OS_TMR  *ptmr1;
    OS_TMR  *ptmr2;


    OS_MemClr((INT8U *)&OSTmrTbl[0],      sizeof(OSTmrTbl));            /* Clear all the TMRs                         */
    OS_MemClr((INT8U *)&OSTmrWheelTbl[0], sizeof(OSTmrWheelTbl));       /* Clear the timer wheel                      */
    /*除了最后一个，为timer创建链表*/
    for (ix = 0u; ix < (OS_TMR_CFG_MAX - 1u); ix++) {                   /* Init. list of free TMRs                    */
        ix_next = ix + 1u;
        ptmr1 = &OSTmrTbl[ix];
        ptmr2 = &OSTmrTbl[ix_next];
        ptmr1->OSTmrType    = OS_TMR_TYPE;
        ptmr1->OSTmrState   = OS_TMR_STATE_UNUSED;                      /* Indicate that timer is inactive            */
        ptmr1->OSTmrNext    = (void *)ptmr2;                            /* Link to next timer                         */
#if OS_TMR_CFG_NAME_EN > 0u
        ptmr1->OSTmrName    = (INT8U *)(void *)"?";
#endif
    }
    ptmr1               = &OSTmrTbl[ix];
    ptmr1->OSTmrType    = OS_TMR_TYPE;
    ptmr1->OSTmrState   = OS_TMR_STATE_UNUSED;                          /* Indicate that timer is inactive            */
    ptmr1->OSTmrNext    = (void *)0;                                    /* Last OS_TMR                                */
#if OS_TMR_CFG_NAME_EN > 0u
    ptmr1->OSTmrName    = (INT8U *)(void *)"?";
#endif
    OSTmrTime           = 0u;                                           /*对一些timer的全局变量进行赋值*/
    OSTmrUsed           = 0u;
    OSTmrFree           = OS_TMR_CFG_MAX;
    OSTmrFreeList       = &OSTmrTbl[0];
    OSTmrSem            = OSSemCreate(1u);                             /*创建timer的信号量，TmrLock使用不知道干嘛*/
    OSTmrSemSignal      = OSSemCreate(0u);                             /*并未信号量赋初始值*/

#if OS_EVENT_NAME_EN > 0u                                               /* Assign names to semaphores                 */
    OSEventNameSet(OSTmrSem,       (INT8U *)(void *)"uC/OS-II TmrLock",   &err);
    OSEventNameSet(OSTmrSemSignal, (INT8U *)(void *)"uC/OS-II TmrSignal", &err);
#endif

    OSTmr_InitTask();
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                INITIALIZE THE TIMER MANAGEMENT TASK
*
* Description: This function is called by OSTmrInit() to create the timer management task.
*                               * Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  void  OSTmr_InitTask (void)
{
#if OS_TASK_NAME_EN > 0u
    INT8U  err;
#endif


#if OS_TASK_CREATE_EXT_EN > 0u    /*是不是使用创建任务扩展函数*/
    #if OS_STK_GROWTH == 1u       /*根据堆栈的方向不同创建不同的任务，适应不同的CPU*/
    (void)OSTaskCreateExt(OSTmr_Task,
                          (void *)0,                                       /* No arguments passed to OSTmrTask()      */
                          &OSTmrTaskStk[OS_TASK_TMR_STK_SIZE - 1u],        /* Set Top-Of-Stack                        */
                          OS_TASK_TMR_PRIO,                                /*任务的优先级*/
                          OS_TASK_TMR_ID,                                  /*任务ID在ucosII.h有定义*/
                          &OSTmrTaskStk[0],                                /* Set Bottom-Of-Stack                     */
                          OS_TASK_TMR_STK_SIZE,
                          (void *)0,                                       /* No TCB extension                        */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);      /* Enable stack checking + clear stack     */
    #else
    (void)OSTaskCreateExt(OSTmr_Task,
                          (void *)0,                                       /* No arguments passed to OSTmrTask()      */
                          &OSTmrTaskStk[0],                                /* Set Top-Of-Stack                        */
                          OS_TASK_TMR_PRIO,
                          OS_TASK_TMR_ID,
                          &OSTmrTaskStk[OS_TASK_TMR_STK_SIZE - 1u],        /* Set Bottom-Of-Stack                     */
                          OS_TASK_TMR_STK_SIZE,
                          (void *)0,                                       /* No TCB extension                        */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);      /* Enable stack checking + clear stack     */
    #endif
#else                           /*使用普通创建任务函数*/
    #if OS_STK_GROWTH == 1u
    (void)OSTaskCreate(OSTmr_Task,
                       (void *)0,
                       &OSTmrTaskStk[OS_TASK_TMR_STK_SIZE - 1u],
                       OS_TASK_TMR_PRIO);
    #else
    (void)OSTaskCreate(OSTmr_Task,
                       (void *)0,
                       &OSTmrTaskStk[0],
                       OS_TASK_TMR_PRIO);
    #endif
#endif

#if OS_TASK_NAME_EN > 0u
    OSTaskNameSet(OS_TASK_TMR_PRIO, (INT8U *)(void *)"uC/OS-II Tmr", &err);
#endif
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                 INSERT A TIMER INTO THE TIMER WHEEL
*
* Description: This function is called to insert the timer into the timer wheel.  The timer is always 
*              inserted at the beginning of the list.
*
* Arguments  : ptmr          Is a pointer to the timer to insert.
*
*              type          Is either:
*                               OS_TMR_LINK_PERIODIC    Means to re-insert the timer after a period expired
*                               OS_TMR_LINK_DLY         Means to insert    the timer the first time
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  void  OSTmr_Link (OS_TMR  *ptmr,
                          INT8U    type)
{
    OS_TMR       *ptmr1;
    OS_TMR_WHEEL *pspoke;
    INT16U        spoke;

    
    ptmr->OSTmrState = OS_TMR_STATE_RUNNING;                       /*赋值开始运行timer*/
    if (type == OS_TMR_LINK_PERIODIC) {                            /* Determine when timer will expire                */
        ptmr->OSTmrMatch = ptmr->OSTmrPeriod + OSTmrTime;          /*re_insert连续定时的时间计算,OSTmrMatch表示计时到的时间*/
    } else {
        if (ptmr->OSTmrDly == 0u) {                               /*第一次insert 定时器到timer wheel*/
            ptmr->OSTmrMatch = ptmr->OSTmrPeriod + OSTmrTime;     /*如果Dly==0 则使用Period*/
        } else {
            ptmr->OSTmrMatch = ptmr->OSTmrDly    + OSTmrTime;
        }
    }
    spoke  = (INT16U)(ptmr->OSTmrMatch % OS_TMR_CFG_WHEEL_SIZE);   /*对定时进行分组*/
    pspoke = &OSTmrWheelTbl[spoke];

    if (pspoke->OSTmrFirst == (OS_TMR *)0) {                       /* Link into timer wheel                           */
        pspoke->OSTmrFirst   = ptmr;                               /*该分组原来没有timer*/
        ptmr->OSTmrNext      = (OS_TMR *)0;
        pspoke->OSTmrEntries = 1u;                                  /*标记只有一个timer*/
    } else {
        ptmr1                = pspoke->OSTmrFirst;                 /* Point to first timer in the spoke               */
        pspoke->OSTmrFirst   = ptmr;                               /*将新增加的timer链接到表头*/
        ptmr->OSTmrNext      = (void *)ptmr1;
        ptmr1->OSTmrPrev     = (void *)ptmr;
        pspoke->OSTmrEntries++;
    }
    ptmr->OSTmrPrev = (void *)0;                                   /* Timer always inserted as first node in list     */
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                 REMOVE A TIMER FROM THE TIMER WHEEL
*
* Description: This function is called to remove the timer from the timer wheel.
*
* Arguments  : ptmr          Is a pointer to the timer to remove.
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  void  OSTmr_Unlink (OS_TMR *ptmr)
{
    OS_TMR        *ptmr1;
    OS_TMR        *ptmr2;
    OS_TMR_WHEEL  *pspoke;
    INT16U         spoke;


    spoke  = (INT16U)(ptmr->OSTmrMatch % OS_TMR_CFG_WHEEL_SIZE);/*确定timer的分组*/
    pspoke = &OSTmrWheelTbl[spoke];

    if (pspoke->OSTmrFirst == ptmr) {                       /* See if timer to remove is at the beginning of list     */
        ptmr1              = (OS_TMR *)ptmr->OSTmrNext;
        pspoke->OSTmrFirst = (OS_TMR *)ptmr1;
        if (ptmr1 != (OS_TMR *)0) {
            ptmr1->OSTmrPrev = (void *)0;                   /*如果不等于0，将前指针指向0*/
        }
    } else {
        ptmr1            = (OS_TMR *)ptmr->OSTmrPrev;       /* Remove timer from somewhere in the list                */
        ptmr2            = (OS_TMR *)ptmr->OSTmrNext;
        ptmr1->OSTmrNext = ptmr2;
        if (ptmr2 != (OS_TMR *)0) {
            ptmr2->OSTmrPrev = (void *)ptmr1;
        }
    }
    ptmr->OSTmrState = OS_TMR_STATE_STOPPED;               /*标记定时器为stop*/
    ptmr->OSTmrNext  = (void *)0;
    ptmr->OSTmrPrev  = (void *)0;
    pspoke->OSTmrEntries--;
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                        TIMER MANAGEMENT TASK
*
* Description: This task is created by OSTmrInit().
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if OS_TMR_EN > 0u
static  void  OSTmr_Task (void *p_arg)
{
    INT8U            err;
    OS_TMR          *ptmr;
    OS_TMR          *ptmr_next;
    OS_TMR_CALLBACK  pfnct;
    OS_TMR_WHEEL    *pspoke;
    INT16U           spoke;


    p_arg = p_arg;                                               /* Prevent compiler warning for not using 'p_arg'    */
    for (;;) {
        OSSemPend(OSTmrSemSignal, 0u, &err);                     /* Wait for signal indicating time to update timers  */
        OSSchedLock();                                           /*Lock调度器*/
        OSTmrTime++;                                             /* Increment the current time                        */
        spoke  = (INT16U)(OSTmrTime % OS_TMR_CFG_WHEEL_SIZE);    /* Position on current timer wheel entry             */
        pspoke = &OSTmrWheelTbl[spoke];
        ptmr   = pspoke->OSTmrFirst;
        while (ptmr != (OS_TMR *)0) {
            ptmr_next = (OS_TMR *)ptmr->OSTmrNext;               /* Point to next timer to update because current ... */
                                                                 /* ... timer could get unlinked from the wheel.      */
            if (OSTmrTime == ptmr->OSTmrMatch) {                 /* Process each timer that expires                   */
                OSTmr_Unlink(ptmr);                              /* Remove from current wheel spoke                   */
                if (ptmr->OSTmrOpt == OS_TMR_OPT_PERIODIC) {     /*是不是连续计数*/
                    OSTmr_Link(ptmr, OS_TMR_LINK_PERIODIC);      /* Recalculate new position of timer in wheel        */
                } else {
                    ptmr->OSTmrState = OS_TMR_STATE_COMPLETED;   /* Indicate that the timer has completed             */
                }
                pfnct = ptmr->OSTmrCallback;                     /* Execute callback function if available            */
                if (pfnct != (OS_TMR_CALLBACK)0) {
                    (*pfnct)((void *)ptmr, ptmr->OSTmrCallbackArg);/*调用回调函数*/
                }
            }
            ptmr = ptmr_next;
        }
        OSSchedUnlock();
    }
}
#endif
