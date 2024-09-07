/*
 * Application definition for miscelanous tasks
 * Description:
 */
#include "app_misc.h"

#if (APP_TASK_MATH>0u)
OS_TCB App_TaskMathTCB;
CPU_STK_SIZE App_TaskMathStk[APP_CFG_TASK_MATH_STK_SIZE];
#endif

#if (APP_TASK_MATH>0u)

void APP_Math_Init(OS_ERR *err) {

	OSTaskCreate((OS_TCB*) &App_TaskMathTCB, (CPU_CHAR*) "App Task Math",
			(OS_TASK_PTR) APP_Task_math, (void*) 0,
			(OS_PRIO) APP_CFG_TASK_MATH_PRIO, (CPU_STK*) &App_TaskMathStk[0],
			(CPU_STK) (APP_CFG_TASK_MATH_STK_SIZE / 10),
			(CPU_STK_SIZE) APP_CFG_TASK_MATH_STK_SIZE, (OS_MSG_QTY) 0,
			(OS_TICK) 0, (void*) 0,
			(OS_OPT) (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR ),
			(OS_ERR*) err
	);
	return;
}

void APP_Task_math(void *p_arg) {

	OS_ERR err;
	volatile CPU_INT32U a[MATH_OPS_LENGTH], b[MATH_OPS_LENGTH];
	volatile CPU_INT32U c[MATH_OPS_LENGTH];

	(void) p_arg;
	CPU_INT16U i;

	for (i = 0; i < MATH_OPS_LENGTH; i++) {
		a[i] = i;
		b[i] = i;
		c[i] = i;
	}
	
	/*it is the dummy math task*/
	while (DEF_TRUE) {
		for (i = 0; i < MATH_OPS_LENGTH; i++) {
			c[i] |= a[i] / b[i];
			c[i] |= a[i] * b[i];
			c[i] |= a[i] + b[i];
		}


		/*delay*/
		OSTimeDlyHMSM(0, 0, 0, 50, OS_OPT_TIME_TIMEOUT, &err);

	}
	return;
}
#endif
