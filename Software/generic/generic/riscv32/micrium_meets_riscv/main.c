/*----------------------------------------------------------------------------
 * Name:    main.c
 * Purpose: Starting point of OS and application code
 * Note(s):
 * Author(s): Franout, Francesco Angione (francesco.angione@polito.it)
 *----------------------------------------------------------------------------
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2020 Politecnico di Torino. All rights reserved.
 *----------------------------------------------------------------------------*/

#include "includes.h"
#include "app_cfg.h"

#define PRINT_ERROR_SYNDROME(syndrome)  																				\
	do                                                                                                                  \
	{                                                                                                                  	\
		for (int i = 0; i < 10; i++)                                                                                   	\
		{                                                                                                              	\
			asm volatile("li x6,0xc1a0");                                                                              	\
			asm volatile("li x7,0xc1a0");                                                                              	\
			asm volatile("li x8,0xc1a0");                                                                              	\
			asm volatile("mv x9,%0" : : "r"(syndrome));                                                                 \
			asm volatile("mv x10,%0" : : "r"(syndrome));                                                                \
			asm volatile("li x11,0xc1a0");                                                                             	\
			asm volatile("li x12,0xc1a0");                                                                             	\
		}                                                                                                              	\
		exit(syndrome);																										\
	} while (0)
/*
 *********************************************************************************************************
 *                                       APPLICATION GLOBALS
 *********************************************************************************************************
 */
static OS_TCB App_TaskStartTCB;
static CPU_STK_SIZE App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];


/*
 *********************************************************************************************************
 *                                      LOCAL FUNCTION PROTOTYPES
 *********************************************************************************************************
 */
static void App_ObjCreate(void);
static void App_TaskCreate(void);
static void App_TaskStart(void *p_arg);


/*
 *
 * Application entry point.
 */


int main(void) {
	OS_ERR err;
	BSP_Init();
	BSP_IntInit(); /*init interrupts*/
	CPU_IntDis(); /*disable interrutpts*/
	CPU_Init();
	Mem_Init(); /*Initialize the memory heap pool */
	Math_Init(); /*Initialize the math random seed functions */

	OSInit(&err); /* Initialize "uC/OS-III, The Real-Time Kernel"*/
	if (err != OS_ERR_NONE) {
		while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
		}
			
	}

#if (OS_CFG_DBG_EN > 0u) /*for checking the OS configuration*/
	OS_Dbg_Init();
#endif
	App_OS_SetAllHooks(); /* Set all applications hooks      */
	
	OSTaskCreate((OS_TCB*) &App_TaskStartTCB, /* Create the start task     */
	(CPU_CHAR*) "App Task Start", (OS_TASK_PTR) App_TaskStart, (void*) 0,
			(OS_PRIO) APP_CFG_TASK_START_PRIO, (CPU_STK*) &App_TaskStartStk[0],
			(CPU_STK) ((APP_CFG_TASK_START_STK_SIZE * 10) / 100),
			(CPU_STK_SIZE) APP_CFG_TASK_START_STK_SIZE, (OS_MSG_QTY) 0,
			(OS_TICK) 0, (void*) 0,
			(OS_OPT) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR ),
			(OS_ERR*) &err
	);
	if (err != OS_ERR_NONE) {
		while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
		}
	}

#if (OS_TASK_NAME_EN > 0u)
    OSTaskNameSet(APP_CFG_TASK_START_PRIO, "Start", &err);
#endif
	/*
	 * Gotta Catch 'Em All
	 *
	 * */
	OSStart(&err); /* Start the multitasking */
	while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
		}
	return err;


}

/*
 *********************************************************************************************************
 *                                          App_TaskStart()
 *
 * Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
 *               initialize the ticker only once multitasking has started.
 *
 * Argument(s) : p_arg   is the argument passed to 'App_TaskStart()' by 'OSTaskCreate()'.
 *
 * Return(s)   : none.
 *
 * Caller(s)   : This is a task.
 *
 * Notes       : (1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
 *                   used.  The compiler should not generate any code for this statement.
 *********************************************************************************************************
 */

static void App_TaskStart(void *p_arg) {
	OS_ERR os_err = OS_ERR_NONE;
	volatile CLK_TS_SEC ts_sec = 0; /* avoiding optimizations*/
	volatile OS_CPU_USAGE cpu_usage = 0; /*avoiding optimizations*/
	volatile OS_CPU_USAGE cpu_usage_pk = 0; /*avoiding optimizations*/
	CLK_DATE_TIME date_time;
	CPU_BOOLEAN valid = DEF_OK;
	CPU_CHAR str[CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC_LEN + 1];
	CLK_ERR err = CLK_ERR_NONE;

	(void) p_arg; /* See Note #1                                          */

	OS_TRACE_INIT(); /* Initialize Trace recorder                            */
	/*Initialize uC-CLK module */

	Clk_Init(&err);
	if (err != CLK_ERR_NONE) {
		while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
		}
	}

	/* Set the initial time for sw maintained timestamp
	 * This data corresponds to the exact date in which
	 * the first version of uC-OSIII was up and running
	 *  on bernina
	 * */
	valid = Clk_DateTimeMake(&date_time, 2021, 01, 29, 11, 11, 11,
	CLK_CFG_TZ_DFLT_SEC);
	if (valid != DEF_OK) {
		while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
		}
	}
	valid = Clk_SetDateTime(&date_time);
	if (valid != DEF_OK) {
		while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
		}
	}



#ifdef CPU_CFG_INT_DIS_MEAS_EN
	CPU_IntDisMeasMaxCurReset();
#endif

	App_ObjCreate(); /* Create Applicaiton kernel objects                    */
	/*following the linux task creation, all the tasks are child of the init task*/
	App_TaskCreate();/* Create Application tasks                             */

	/* Enable the tick timer and interrupt  */
	BSP_OS_Tick_Init(); 
	BSP_OS_TickEnable(); 
	CPU_IntEn();/*enable interrupts*/

	while (DEF_TRUE) {
#if OS_CFG_STAT_TASK_EN > 0u
	/*it sycnhronizes with tick task */
	OSStatTaskCPUUsageInit(&os_err); /* Compute CPU capacity with no task running            */
#endif
		(CLK_TS_SEC) ts_sec;

		/*get date time data structure*/
		valid = Clk_GetDateTime(&date_time);
		if (valid != DEF_OK) {
			while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
		}	
		}
		/*printing functions for timestamp*/
		valid = Clk_DateTimeToStr(&date_time,
		CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC, str,
		CLK_STR_FMT_YYYY_MM_DD_HH_MM_SS_UTC_LEN + 1);
		if (valid != DEF_OK) {
			while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
			}
		}
		/* getting timestamp */
		ts_sec = Clk_GetTS(); //or Clk_DateTimeToTS(&ts_sec, &date_time)

#if OS_CFG_STAT_TASK_EN > 0u
		/*getting cpu usage in percentage
		 *
		 * CPU_utilization= 100- (100*OSStatTaskCtr)/(OSStatCtrMax)
		 *
		 * */
		cpu_usage = OSStatTaskCPUUsage;
		(OS_CPU_USAGE) cpu_usage;
		cpu_usage_pk = OSStatTaskCPUUsageMax;
		(OS_CPU_USAGE) cpu_usage_pk;
#endif /*OS_CFG_STAT_TASK_EN*/
		OSTimeDlyHMSM(0, 0, 0, 2, OS_OPT_TIME_TIMEOUT, &os_err);

	}

}
/*
 *********************************************************************************************************
 *                                      App_ObjCreate()
 *
 * Description:  Creates the application kernel objects.
 *
 * Argument(s) :  none.
 *
 * Return(s)   :  none.
 *
 * Caller(s)   :  App_TaskStart().
 *
 * Note(s)     :  none.
 *********************************************************************************************************
 */

static void App_ObjCreate(void) {

}

/*
 *********************************************************************************************************
 *                                      App_TaskCreate()
 *
 * Description :  This function creates the application tasks.
 *
 * Argument(s) :  none.
 *
 * Return(s)   :  none.
 *
 * Caller(s)   :  App_TaskStart().
 *
 * Note(s)     :  none.
 *********************************************************************************************************
 */

static void App_TaskCreate(void) {
	OS_ERR err =  OS_ERR_NONE;
	#if (APP_TASK_MATH>0u)
	APP_Math_Init(&err);
	if (err != OS_ERR_NONE) {
		while (DEF_TRUE){
			PRINT_ERROR_SYNDROME(err);
		}
	}
	#endif /*APP_TASK_MATH*/
}
