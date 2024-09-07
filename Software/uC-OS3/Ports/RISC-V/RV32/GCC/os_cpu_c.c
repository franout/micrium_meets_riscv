/*
*********************************************************************************************************
*                                              uC/OS-III
*                                        The Real-Time Kernel
*
*                    Copyright 2009-2022 Silicon Laboratories Inc. www.silabs.com
*
*                                 SPDX-License-Identifier: APACHE-2.0
*
*               This software is subject to an open source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                              RISC-V PORT
*
* File      : os_cpu_c.c
* Version   : V3.08.02
*********************************************************************************************************
* For       : RISC-V RV32
* Toolchain : GNU C Compiler
*********************************************************************************************************
* Note(s)   : Hardware FP is not supported.
*********************************************************************************************************
*/

#define OS_CPU_GLOBALS

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const CPU_CHAR *os_cpu_c__c = "$Id: $";
#endif

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include "../../../../Source/os.h"

#ifdef __COMET_SIMULATOR__
#include <stdlib.h>
#endif /*__COMET_SIMULATOR__*/

#include "cpu.h"
#include "riscv-csr.h"
#include "riscv-interrupts.h"
#include "timer.h"

/*
*********************************************************************************************************
*                                           LOCAL DEFINES
*********************************************************************************************************
*/
#define PRINT_END()                                                                                                    \
	do                                                                                                                 \
	{                                                                                                                  \
		for (int i = 0; i < 10; i++)                                                                                   \
		{                                                                                                              \
			asm volatile("li x6,0xc1a0");                                                                              \
			asm volatile("li x7,0xE41d");                                                                              \
			asm volatile("li x8,0xc1a0");                                                                              \
			asm volatile("li x9,0xE41d");                                                                              \
			asm volatile("li x10,0xc1a0");                                                                             \
			asm volatile("li x11,0xE41d");                                                                             \
			asm volatile("li x12,0xc1a0");                                                                             \
			asm volatile("li x13,0xE41d");                                                                             \
			asm volatile("li x14,0xc1a0");                                                                             \
			asm volatile("li x15,0xE41d");                                                                             \
			asm volatile("li x16,0xc1a0");                                                                             \
			asm volatile("li x17,0xE41d");                                                                             \
			asm volatile("li x18,0xc1a0");                                                                             \
			asm volatile("li x19,0xE41d");                                                                             \
			asm volatile("li x20,0xc1a0");                                                                             \
			asm volatile("li x21,0xE41d");                                                                             \
		}                                                                                                              \
	} while (0)
#define PRINT_CAUSE_ON_REGS(cause)                                                                                     \
	do                                                                                                                 \
	{                                                                                                                  \
		for (int i = 0; i < 10; i++)                                                                                   \
		{                                                                                                              \
			asm volatile("li x6,0xc1a0");                                                                              \
			asm volatile("li x7,0xc1a0");                                                                              \
			asm volatile("li x8,0xc1a0");                                                                              \
			asm volatile("mv x9,%0" : : "r"(cause));                                                                   \
			asm volatile("mv x10,%0" : : "r"(cause));                                                                  \
			asm volatile("li x11,0xc1a0");                                                                             \
			asm volatile("li x12,0xc1a0");                                                                             \
		}                                                                                                              \
	} while (0)

#define HANDLER_PROLOGUE()                                                                                             \
	do                                                                                                                 \
	{                                                                                                                  \
		asm volatile("addi   sp, sp, -4 * 32");                                                                        \
		asm volatile("sw     x31,  29 * 4(sp)");                                                                       \
		asm volatile("sw     x30,  28 * 4(sp)");                                                                       \
		asm volatile("sw     x29,  27 * 4(sp)");                                                                       \
		asm volatile("sw     x28,  26 * 4(sp)");                                                                       \
		asm volatile("sw     x27,  25 * 4(sp)");                                                                       \
		asm volatile("sw     x26,  24 * 4(sp)");                                                                       \
		asm volatile("sw     x25,  23 * 4(sp)");                                                                       \
		asm volatile("sw     x24,  22 * 4(sp)");                                                                       \
		asm volatile("sw     x23,  21 * 4(sp)");                                                                       \
		asm volatile("sw     x22,  20 * 4(sp)");                                                                       \
		asm volatile("sw     x21,  19 * 4(sp)");                                                                       \
		asm volatile("sw     x20,  18 * 4(sp)");                                                                       \
		asm volatile("sw     x19,  17 * 4(sp)");                                                                       \
		asm volatile("sw     x18,  16 * 4(sp)");                                                                       \
		asm volatile("sw     x17,  15 * 4(sp)");                                                                       \
		asm volatile("sw     x16,  14 * 4(sp)");                                                                       \
		asm volatile("sw     x15,  13 * 4(sp)");                                                                       \
		asm volatile("sw     x14,  12 * 4(sp)");                                                                       \
		asm volatile("sw     x13,  11 * 4(sp)");                                                                       \
		asm volatile("sw     x12,  10 * 4(sp)");                                                                       \
		asm volatile("sw     x11,  9 * 4(sp)");                                                                       \
		asm volatile("sw     x10,  8 * 4(sp)");                                                                        \
		asm volatile("sw     x9,   7 * 4(sp)");                                                                        \
		asm volatile("sw     x8,   6 * 4(sp)");                                                                        \
		asm volatile("sw     x7,   5 * 4(sp)");                                                                        \
		asm volatile("sw     x6,   4 * 4(sp)");                                                                        \
		asm volatile("sw     x5,   3 * 4(sp)");                                                                        \
		asm volatile("sw     x4,   2 * 4(sp)");                                                                        \
		asm volatile("sw     x3,   1 * 4(sp)");                                                                        \
		asm volatile("sw     x1,   0 * 4(sp)");                                                                        \
		asm volatile("csrrs  x0,mstatus,x30");                                                                             \
		asm volatile("sw   x30, 30 *4 (sp)");                                                                          \
		asm volatile("csrrs   x0,mepc,x31");                                                                             \
		asm volatile("sw     x31,    31*4(sp)");                                                                       \
	} while (0)

#define HANDLER_EPILOGUE()                                                                                             \
	do                                                                                                                 \
	{                                                                                                                  \
		asm volatile("lw     x31,    31*4(sp)");                                                                       \
		asm volatile("csrrw  x31, mepc, x31");                                                                          \
		asm volatile("lw     x30,    30*4(sp)");                                                                       \
		asm volatile("csrrw  x30, mstatus, x30");                                                                       \
		asm volatile("lw     x1,   0* 4(sp)");                                                                         \
		asm volatile("lw     x3,   1 * 4(sp)");                                                                        \
		asm volatile("lw     x4,   2 * 4(sp)");                                                                        \
		asm volatile("lw     x5,   3 * 4(sp)");                                                                        \
		asm volatile("lw     x6,   4 * 4(sp)");                                                                        \
		asm volatile("lw     x7,   5 * 4(sp)");                                                                        \
		asm volatile("lw     x8,   6 * 4(sp)");                                                                        \
		asm volatile("lw     x9,   7 * 4(sp)");                                                                        \
		asm volatile("lw     x10,  8 * 4(sp)");                                                                        \
		asm volatile("lw     x11,  9 * 4(sp)");                                                                       \
		asm volatile("lw     x12,  10 * 4(sp)");                                                                       \
		asm volatile("lw     x13,  11 * 4(sp)");                                                                       \
		asm volatile("lw     x14,  12 * 4(sp)");                                                                       \
		asm volatile("lw     x15,  13 * 4(sp)");                                                                       \
		asm volatile("lw     x16,  14 * 4(sp)");                                                                       \
		asm volatile("lw     x17,  15 * 4(sp)");                                                                       \
		asm volatile("lw     x18,  16 * 4(sp)");                                                                       \
		asm volatile("lw     x19,  17 * 4(sp)");                                                                       \
		asm volatile("lw     x20,  18 * 4(sp)");                                                                       \
		asm volatile("lw     x21,  19 * 4(sp)");                                                                       \
		asm volatile("lw     x22,  20 * 4(sp)");                                                                       \
		asm volatile("lw     x23,  21 * 4(sp)");                                                                       \
		asm volatile("lw     x24,  22 * 4(sp)");                                                                       \
		asm volatile("lw     x25,  23 * 4(sp)");                                                                       \
		asm volatile("lw     x26,  24 * 4(sp)");                                                                       \
		asm volatile("lw     x27,  25 * 4(sp)");                                                                       \
		asm volatile("lw     x28,  26 * 4(sp)");                                                                       \
		asm volatile("lw     x29,  27 * 4(sp)");                                                                       \
		asm volatile("lw     x30,  28 * 4(sp)");                                                                       \
		asm volatile("lw     x31,  29 * 4(sp)");                                                                       \
		asm volatile("addi   sp, sp, 4 * 32");                                                                         \
	} while (0)
/*
*********************************************************************************************************
*                                     EXTERNAL C LANGUAGE LINKAGE
*
* Note(s) : (1) C++ compilers MUST 'extern'ally declare ALL C function prototypes & variable/object
*               declarations for correct C language linkage.
*********************************************************************************************************
*/

#ifdef __cplusplus
extern "C"
{ /* See Note #1.                                       */
#endif

	/*
	*********************************************************************************************************
	*                                             IDLE TASK HOOK
	*
	* Description: This function is called by the idle task.  This hook has been added to allow you to do
	*              such things as STOP the CPU to conserve power.
	*
	* Arguments  : None.
	*
	* Note(s)    : None.
	*********************************************************************************************************
	*/

	void OSIdleTaskHook(void)
	{
#if OS_CFG_APP_HOOKS_EN > 0u
		if (OS_AppIdleTaskHookPtr != (OS_APP_HOOK_VOID)0)
		{
			(*OS_AppIdleTaskHookPtr)();
		}
#endif

#ifdef __COMET_SIMULATOR__
		static CPU_INT32U counter = 0;

		if (counter > MAX_HYPERPERIOD_REPS)
		{
			PRINT_END();
			exit(0);
		}
		counter++;

#endif /*__COMET_SIMULATOR__*/
	}

	/*
	*********************************************************************************************************
	*                                       OS INITIALIZATION HOOK
	*
	* Description: This function is called by OSInit() at the beginning of OSInit().
	*
	* Arguments  : None.
	*
	* Note(s)    : None.
	*********************************************************************************************************
	*/

	void OSInitHook(void)
	{
	}

/*
*********************************************************************************************************
*                                           REDZONE HIT HOOK
*
* Description: This function is called when a task's stack overflowed.
*
* Arguments  : p_tcb        Pointer to the task control block of the offending task. NULL if ISR.
*
* Note(s)    : None.
*********************************************************************************************************
*/
#if (OS_CFG_TASK_STK_REDZONE_EN > 0u)
	void OSRedzoneHitHook(OS_TCB *p_tcb)
	{
#if OS_CFG_APP_HOOKS_EN > 0u
		if (OS_AppRedzoneHitHookPtr != (OS_APP_HOOK_TCB)0)
		{
			(*OS_AppRedzoneHitHookPtr)(p_tcb);
		}
		else
		{
			CPU_SW_EXCEPTION(;);
		}
#else
		(void)p_tcb; /* Prevent compiler warning                             */
		CPU_SW_EXCEPTION(;);
#endif
	}
#endif

	/*
	*********************************************************************************************************
	*                                         STATISTIC TASK HOOK
	*
	* Description: This function is called every second by uC/OS-III's statistics task.  This allows your
	*              application to add functionality to the statistics task.
	*
	* Arguments  : None.
	*
	* Note(s)    : None.
	*********************************************************************************************************
	*/

	void OSStatTaskHook(void)
	{
#if OS_CFG_APP_HOOKS_EN > 0u
		if (OS_AppStatTaskHookPtr != (OS_APP_HOOK_VOID)0)
		{
			(*OS_AppStatTaskHookPtr)();
		}
#endif
	}

	/*
	*********************************************************************************************************
	*                                          TASK CREATION HOOK
	*
	* Description: This function is called when a task is created.
	*
	* Arguments  : p_tcb        Pointer to the task control block of the task being created.
	*
	* Note(s)    : None.
	*********************************************************************************************************
	*/

	void OSTaskCreateHook(OS_TCB *p_tcb)
	{
#if OS_CFG_APP_HOOKS_EN > 0u
		if (OS_AppTaskCreateHookPtr != (OS_APP_HOOK_TCB)0)
		{
			(*OS_AppTaskCreateHookPtr)(p_tcb);
		}
#else
	(void)p_tcb; /* Prevent compiler warning                             */
#endif
	}

	/*
	*********************************************************************************************************
	*                                          TASK DELETION HOOK
	*
	* Description: This function is called when a task is deleted.
	*
	* Arguments  : p_tcb        Pointer to the task control block of the task being deleted.
	*
	* Note(s)    : None.
	*********************************************************************************************************
	*/

	void OSTaskDelHook(OS_TCB *p_tcb)
	{
#if OS_CFG_APP_HOOKS_EN > 0u
		if (OS_AppTaskDelHookPtr != (OS_APP_HOOK_TCB)0)
		{
			(*OS_AppTaskDelHookPtr)(p_tcb);
		}
#else
	(void)p_tcb; /* Prevent compiler warning                             */
#endif
	}

	/*
	*********************************************************************************************************
	*                                           TASK RETURN HOOK
	*
	* Description: This function is called if a task accidentally returns.  In other words, a task should
	*              either be an infinite loop or delete itself when done.
	*
	* Arguments  : p_tcb        Pointer to the task control block of the task that is returning.
	*
	* Note(s)    : None.
	*********************************************************************************************************
	*/

	void OSTaskReturnHook(OS_TCB *p_tcb)
	{
#if OS_CFG_APP_HOOKS_EN > 0u
		if (OS_AppTaskReturnHookPtr != (OS_APP_HOOK_TCB)0)
		{
			(*OS_AppTaskReturnHookPtr)(p_tcb);
		}
#else
	(void)p_tcb; /* Prevent compiler warning                             */
#endif
	}

	/*
	*********************************************************************************************************
	*                                        INITIALIZE A TASK'S STACK
	*
	* Description: This function is called by OSTaskCreate() to initialize the stack frame of the task being
	*              created. This function is highly processor specific.
	*
	* Arguments  : p_task       Pointer to the task entry point address.
	*
	*              p_arg        Pointer to a user supplied data area that will be passed to the task
	*                               when the task first executes.
	*
	*              p_stk_base   Pointer to the base address of the stack.
	*
	*              stk_size     Size of the stack, in number of CPU_STK elements.
	*
	*              opt          Options used to alter the behavior of OS_Task_StkInit().
	*                            (see OS.H for OS_TASK_OPT_xxx).
	*
	* Returns    : Always returns the location of the new top-of-stack once the processor registers have
	*              been placed on the stack in the proper order.
	*
	* Note(s)    : 1) Interrupts are enabled when task starts executing.
	*
	*              2) There is no need to save register x0 since it is a hard-wired zero.
	*
	*              3) RISC-V calling convention register usage:
	*
	*                    +-------------+-------------+----------------------------------+
	*                    |  Register   |   ABI Name  | Description                      |
	*                    +-------------+-------------+----------------------------------+
	*                    |  x31 - x28  |   t6 - t3   | Temporaries                      |
	*                    +-------------+-------------+----------------------------------+
	*                    |  x27 - x18  |  s11 - s2   | Saved registers                  |
	*                    +-------------+-------------+----------------------------------+
	*                    |  x17 - x12  |   a7 - a2   | Function arguments               |
	*                    +-------------+-------------+----------------------------------+
	*                    |  x11 - x10  |   a1 - a0   | Function arguments/return values |
	*                    +-------------+-------------+----------------------------------+
	*                    |     x9      |     s1      | Saved register                   |
	*                    +-------------+-------------+----------------------------------+
	*                    |     x8      |    s0/fp    | Saved register/frame pointer     |
	*                    +-------------+-------------+----------------------------------+
	*                    |   x7 - x5   |   t2 - t0   | Temporaries                      |
	*                    +-------------+-------------+----------------------------------+
	*                    |     x4      |     tp      | Thread pointer                   |
	*                    +-------------+-------------+----------------------------------+
	*                    |     x3      |     gp      | Global pointer                   |
	*                    +-------------+-------------+----------------------------------+
	*                    |     x2      |     sp      | Stack pointer                    |
	*                    +-------------+-------------+----------------------------------+
	*                    |     x1      |     ra      | return address                   |
	*                    +-------------+-------------+----------------------------------+
	*                    |     x0      |    zero     | Hard-wired zero                  |
	*                    +-------------+-------------+----------------------------------+
	*
	*********************************************************************************************************
	*/

	CPU_STK *OSTaskStkInit(OS_TASK_PTR p_task, void *p_arg, CPU_STK *p_stk_base, CPU_STK *p_stk_limit,
						   CPU_STK_SIZE stk_size, OS_OPT opt)
	{
		CPU_STK *p_stk;

		(void)p_stk_limit; /* Prevent compiler warning                             */
		(void)opt;

		p_stk = &p_stk_base[stk_size]; /* Load stack pointer and align it to 16-bytes          */
		p_stk = (CPU_STK *)((CPU_STK)(p_stk) & 0xFFFFFFF0u);

		*(--p_stk) = (CPU_STK)p_task;		/* Entry Point                                          */
		*(--p_stk) = CPU_SR_Save();			/*mstatus*/
		*(--p_stk) = (CPU_STK)0x31313131uL; /*x31 t6                                                   */
		*(--p_stk) = (CPU_STK)0x30303030uL; /*x30 t5                                                   */
		*(--p_stk) = (CPU_STK)0x29292929uL; /*x29 t4                                                   */
		*(--p_stk) = (CPU_STK)0x28282828uL; /*x28 t3                                                   */
											/* Saved Registers                                      */
		*(--p_stk) = (CPU_STK)0x27272727uL; /*x27 s11                                                  */
		*(--p_stk) = (CPU_STK)0x26262626uL; /*x26 s10                                                  */
		*(--p_stk) = (CPU_STK)0x25252525uL; /*x25 s9                                                   */
		*(--p_stk) = (CPU_STK)0x24242424uL; /*x24 s8                                                   */
		*(--p_stk) = (CPU_STK)0x23232323uL; /*x23 s7                                                   */
		*(--p_stk) = (CPU_STK)0x22222222uL; /*x22 s6                                                   */
		*(--p_stk) = (CPU_STK)0x21212121uL; /*x21 s5                                                   */
		*(--p_stk) = (CPU_STK)0x20202020uL; /*x20 s4                                                   */
		*(--p_stk) = (CPU_STK)0x19191919uL; /*x19 s3                                                   */
		*(--p_stk) = (CPU_STK)0x18181818uL; /*x18 s2                                                   */
											/* Function Arguments                                   */
		*(--p_stk) = (CPU_STK)0x17171717uL; /*x17 a7                                                   */
		*(--p_stk) = (CPU_STK)0x16161616uL; /*x16 a6                                                   */
		*(--p_stk) = (CPU_STK)0x15151515uL; /*x15 a5                                                   */
		*(--p_stk) = (CPU_STK)0x14141414uL; /*x14 a4                                                   */
		*(--p_stk) = (CPU_STK)0x13131313uL; /*x13 a3                                                   */
		*(--p_stk) = (CPU_STK)0x12121212uL; /*x12 a2                                                   */
											/* Function Arguments/return values                     */
		*(--p_stk) = (CPU_STK)0x11111111uL; /*x11 a1                                                   */
		*(--p_stk) = (CPU_STK)p_arg;		/*x10 a0                                                   */

		*(--p_stk) = (CPU_STK)0x09090909uL;	 /*x9 s1   : Saved register                                */
		*(--p_stk) = (CPU_STK)0x08080808uL;	 /*x8 s0/fp: Saved register/Frame pointer                  */
											 /* Temporary registers                                  */
		*(--p_stk) = (CPU_STK)0x07070707uL;	 /*x7 t2                                                   */
		*(--p_stk) = (CPU_STK)0x06060606uL;	 /*x6 t1                                                   */
		*(--p_stk) = (CPU_STK)0x05050505uL;	 /*x5 t0                                                   */
		*(--p_stk) = (CPU_STK)0x04040404uL;	 /*x4 tp: Thread pointer                                   */
		*(--p_stk) = (CPU_STK)CPU_get_gp();	 /*x3 gp: Global pointer                                   */
		*(--p_stk) = (CPU_STK)OS_TaskReturn; /*x1 ra: return address                                   */

		return (p_stk);
	}

	/*
	*********************************************************************************************************
	*                                          TASK SWITCH HOOK
	*
	* Description: This function is called when a task switch is performed.  This allows you to perform other
	*              operations during a context switch.
	*
	* Arguments  : None.
	*
	* Note(s)    : 1) Interrupts are disabled during this call.
	*              2) It is assumed that the global pointer 'OSTCBHighRdyPtr' points to the TCB of the task
	*                 that will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCurPtr' points
	*                 to the task being switched out (i.e. the preempted task).
	*********************************************************************************************************
	*/

	void OSTaskSwHook(void)
	{
#if OS_CFG_TASK_PROFILE_EN > 0u
		CPU_TS ts;
#endif
#ifdef CPU_CFG_INT_DIS_MEAS_EN
		CPU_TS int_dis_time;
#endif
#if (OS_CFG_TASK_STK_REDZONE_EN > 0u)
		CPU_BOOLEAN stk_status;
#endif

#if OS_CFG_APP_HOOKS_EN > 0u
		if (OS_AppTaskSwHookPtr != (OS_APP_HOOK_VOID)0)
		{
			(*OS_AppTaskSwHookPtr)();
		}
#endif

#if OS_CFG_TASK_PROFILE_EN > 0u
		ts = OS_TS_GET();
		if (OSTCBCurPtr != OSTCBHighRdyPtr)
		{
			OSTCBCurPtr->CyclesDelta = ts - OSTCBCurPtr->CyclesStart;
			OSTCBCurPtr->CyclesTotal += (OS_CYCLES)OSTCBCurPtr->CyclesDelta;
		}

		OSTCBHighRdyPtr->CyclesStart = ts;
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
		int_dis_time = CPU_IntDisMeasMaxCurReset(); /* Keep track of per-task interrupt disable time        */
		if (OSTCBCurPtr->IntDisTimeMax < int_dis_time)
		{
			OSTCBCurPtr->IntDisTimeMax = int_dis_time;
		}
#endif

#if OS_CFG_SCHED_LOCK_TIME_MEAS_EN > 0u
		/* Keep track of per-task scheduler lock time           */
		if (OSTCBCurPtr->SchedLockTimeMax < OSSchedLockTimeMaxCur)
		{
			OSTCBCurPtr->SchedLockTimeMax = OSSchedLockTimeMaxCur;
		}
		OSSchedLockTimeMaxCur = (CPU_TS)0; /* Reset the per-task value                             */
#endif

#if (OS_CFG_TASK_STK_REDZONE_EN > 0u)
		/* Check if stack overflowed.                           */
		stk_status = OSTaskStkRedzoneChk((OS_TCB *)0u);
		if (stk_status != OS_TRUE)
		{
			OSRedzoneHitHook(OSTCBCurPtr);
		}
#endif
	}

	/*
	*********************************************************************************************************
	*                                               TICK HOOK
	*
	* Description: This function is called every tick.
	*
	* Arguments  : None.
	*
	* Note(s)    : 1) This function is assumed to be called from the Tick ISR.
	*********************************************************************************************************
	*/

	void OSTimeTickHook(void)
	{
#if OS_CFG_APP_HOOKS_EN > 0u
		if (OS_AppTimeTickHookPtr != (OS_APP_HOOK_VOID)0)
		{
			(*OS_AppTimeTickHookPtr)();
		}
#endif
#if (CPU_CFG_TS_EN > 0u)
		CPU_TS_Update();
#endif
	}

	/*
	*********************************************************************************************************
	*                                         RISCV IRQ HANDLER
	*
	* Description: Handle the interruptS VIA SOFTWARE
	*
	* Arguments  : None.
	*
	* Note(s)    :
	*********************************************************************************************************
	*/

#pragma GCC push_options
// Force the alignment for mtvec.BASE. A 'C' extension program could be aligned to to bytes.
#pragma GCC optimize("align-functions=4")
	__attribute__((naked)) void riscv_irq_handler(void)
	{
		HANDLER_PROLOGUE();
		uint_xlen_t this_cause = csr_read_mcause();
		/*clean up */
		// csr_write_mcause(0);
		if (this_cause & MCAUSE_INTERRUPT_BIT_MASK)
		{
			/*interrupt*/
			this_cause &= 0xFF;
			// Known exceptions
			switch (this_cause)
			{
			case RISCV_INT_MTI_CODE:
				// Timer exception, keep up the one second tick.
				mtimer_set_raw_time_cmp(MTIMER_MSEC_TO_CLOCKS(1));
				CPU_SR_ALLOC(); /* Allocate storage for CPU status register             */
				CPU_CRITICAL_ENTER();
				OSIntEnter(); /* Tell uC/OS-III that we are starting an ISR           */
				CPU_CRITICAL_EXIT();
				OSTimeTick(); /* Call uC/OS-III's OSTimeTick()                        */
				OSIntExit();  /* Tell uC/OS-III that we are leaving the ISR           */
				break;
			case RISCV_INT_MSI_CODE:
				/*			   a) Disable global interrupts.*/
				/*              b) Clear soft interrupt for hart0.*/
				/*              c) Save the process SP in its TCB, OSTCBCurPtr->StkPtr = SP;*/
				/*              d) Call OSTaskSwHook();*/
				/*              e) Get current high priority, OSPrioCur = OSPrioHighRdy;*/
				/*              f) Get current ready thread TCB, OSTCBCurPtr = OSTCBHighRdyPtr;*/
				/*              g) Get new process SP from TCB, SP = OSTCBHighRdyPtr->StkPtr;*/
				/*              h) Retrieve the address at which exception happened*/
				/*              i) Restore x1-x31 from new process stack; x0 is always zero.*/
				/*              j) Perform exception return which will restore remaining context.*/

				// csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
				csr_clr_bits_mip(0x08);
				asm volatile("sw sp,%0" : : "m"(OSTCBCurPtr->StkPtr));
				OSTaskSwHook();
				OSPrioCur = OSPrioHighRdy;
				OSTCBCurPtr = OSTCBHighRdyPtr;
				asm volatile("lw sp,%0" : : "m"(OSTCBHighRdyPtr->StkPtr));
				// Global interrupt enable
				// csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
				break;
			default:
				PRINT_CAUSE_ON_REGS(this_cause);
				exit(1);
				break;
			}
		}
		else
		{
			PRINT_CAUSE_ON_REGS(this_cause);
			/*exception*/
			exit(1);
		}
		/*reading 0x1800 after the ECALL then you were not in user mode before the trap.
		To enter user mode on the HiFive Rev B, make sure you set MPP = 0, set UIE = 0, MPIE = 1,
		 setup at least one PMP, then MRET. User mode has NO PERMISSIONS by default therefore you
		 must setup a PMP entry before the MRET instruction or y*/
		HANDLER_EPILOGUE();
		asm volatile("mret");
	}
#pragma GCC pop_options

	/*
	*********************************************************************************************************
	*                                   EXTERNAL C LANGUAGE LINKAGE END
	*********************************************************************************************************
	*/

#ifdef __cplusplus
} /* End of 'extern'al C lang linkage.                 */
#endif
