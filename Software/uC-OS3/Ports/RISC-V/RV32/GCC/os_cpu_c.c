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

#include "riscv-csr.h"
#include "riscv-interrupts.h"
#include "timer.h"

/*
*********************************************************************************************************
*                                           LOCAL DEFINES
*********************************************************************************************************
*/

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
		asm volatile("sw     t6,  30 * 4(sp)");                                                                        \
		asm volatile("sw     t5,  29 * 4(sp)");                                                                        \
		asm volatile("sw     t4,  28 * 4(sp)");                                                                        \
		asm volatile("sw     t3,  27 * 4(sp)");                                                                        \
		asm volatile("sw     s11, 26 * 4(sp)");                                                                        \
		asm volatile("sw     s10, 25 * 4(sp)");                                                                        \
		asm volatile("sw     s9,  24 * 4(sp)");                                                                        \
		asm volatile("sw     s8,  23 * 4(sp)");                                                                        \
		asm volatile("sw     s7,  22 * 4(sp)");                                                                        \
		asm volatile("sw     s6,  21 * 4(sp)");                                                                        \
		asm volatile("sw     s5,  20 * 4(sp)");                                                                        \
		asm volatile("sw     s4,  19 * 4(sp)");                                                                        \
		asm volatile("sw     s3,  18 * 4(sp)");                                                                        \
		asm volatile("sw     s2,  17 * 4(sp)");                                                                        \
		asm volatile("sw     a7,  16 * 4(sp)");                                                                        \
		asm volatile("sw     a6,  15 * 4(sp)");                                                                        \
		asm volatile("sw     a5,  14 * 4(sp)");                                                                        \
		asm volatile("sw     a4,  13 * 4(sp)");                                                                        \
		asm volatile("sw     a3,  12 * 4(sp)");                                                                        \
		asm volatile("sw     a2,  11 * 4(sp)");                                                                        \
		asm volatile("sw     a1,  10 * 4(sp)");                                                                        \
		asm volatile("sw     a0,   9 * 4(sp)");                                                                        \
		asm volatile("sw     s1,   8 * 4(sp)");                                                                        \
		asm volatile("sw     s0,   7 * 4(sp)");                                                                        \
		asm volatile("sw     t2,   6 * 4(sp)");                                                                        \
		asm volatile("sw     t1,   5 * 4(sp)");                                                                        \
		asm volatile("sw     t0,   4 * 4(sp)");                                                                        \
		asm volatile("sw     ra,   0 * 4(sp)");                                                                        \
		asm volatile("csrr t0 , mepc");                                                                                \
		asm volatile("sw     t0,    4(sp)");                                                                           \
	} while (0)

#define HANDLER_EPILOGUE()                                                                                             \
	do                                                                                                                 \
	{                                                                                                                  \
		asm volatile("lw     t0,    4(sp)");                                                                           \
		asm volatile("csrr t0 , mepc");                                                                                \
		asm volatile("lw     ra,   0 * 4(sp)");                                                                        \
		asm volatile("lw     t0,   4 * 4(sp)");                                                                        \
		asm volatile("lw     t1,   5 * 4(sp)");                                                                        \
		asm volatile("lw     t2,   6 * 4(sp)");                                                                        \
		asm volatile("lw     s0,   7 * 4(sp)");                                                                        \
		asm volatile("lw     s1,   8 * 4(sp)");                                                                        \
		asm volatile("lw     a0,   9 * 4(sp)");                                                                        \
		asm volatile("lw     a1,  10 * 4(sp)");                                                                        \
		asm volatile("lw     a2,  11 * 4(sp)");                                                                        \
		asm volatile("lw     a3,  12 * 4(sp)");                                                                        \
		asm volatile("lw     a4,  13 * 4(sp)");                                                                        \
		asm volatile("lw     a5,  14 * 4(sp)");                                                                        \
		asm volatile("lw     a6,  15 * 4(sp)");                                                                        \
		asm volatile("lw     a7,  16 * 4(sp)");                                                                        \
		asm volatile("lw     s2,  17 * 4(sp)");                                                                        \
		asm volatile("lw     s3,  18 * 4(sp)");                                                                        \
		asm volatile("lw     s4,  19 * 4(sp)");                                                                        \
		asm volatile("lw     s5,  20 * 4(sp)");                                                                        \
		asm volatile("lw     s6,  21 * 4(sp)");                                                                        \
		asm volatile("lw     s7,  22 * 4(sp)");                                                                        \
		asm volatile("lw     s8,  23 * 4(sp)");                                                                        \
		asm volatile("lw     s9,  24 * 4(sp)");                                                                        \
		asm volatile("lw     s10, 25 * 4(sp)");                                                                        \
		asm volatile("lw     s11, 26 * 4(sp)");                                                                        \
		asm volatile("lw     t3,  27 * 4(sp)");                                                                        \
		asm volatile("lw     t4,  28 * 4(sp)");                                                                        \
		asm volatile("lw     t5,  29 * 4(sp)");                                                                        \
		asm volatile("lw     t6,  30 * 4(sp)");                                                                        \
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
			exit(1);
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

		*(--p_stk) = (CPU_STK)p_task; /* Entry Point                                          */

		*(--p_stk) = (CPU_STK)0x31313131uL; /* t6                                                   */
		*(--p_stk) = (CPU_STK)0x30303030uL; /* t5                                                   */
		*(--p_stk) = (CPU_STK)0x29292929uL; /* t4                                                   */
		*(--p_stk) = (CPU_STK)0x28282828uL; /* t3                                                   */
											/* Saved Registers                                      */
		*(--p_stk) = (CPU_STK)0x27272727uL; /* s11                                                  */
		*(--p_stk) = (CPU_STK)0x26262626uL; /* s10                                                  */
		*(--p_stk) = (CPU_STK)0x25252525uL; /* s9                                                   */
		*(--p_stk) = (CPU_STK)0x24242424uL; /* s8                                                   */
		*(--p_stk) = (CPU_STK)0x23232323uL; /* s7                                                   */
		*(--p_stk) = (CPU_STK)0x22222222uL; /* s6                                                   */
		*(--p_stk) = (CPU_STK)0x21212121uL; /* s5                                                   */
		*(--p_stk) = (CPU_STK)0x20202020uL; /* s4                                                   */
		*(--p_stk) = (CPU_STK)0x19191919uL; /* s3                                                   */
		*(--p_stk) = (CPU_STK)0x18181818uL; /* s2                                                   */
											/* Function Arguments                                   */
		*(--p_stk) = (CPU_STK)0x17171717uL; /* a7                                                   */
		*(--p_stk) = (CPU_STK)0x16161616uL; /* a6                                                   */
		*(--p_stk) = (CPU_STK)0x15151515uL; /* a5                                                   */
		*(--p_stk) = (CPU_STK)0x14141414uL; /* a4                                                   */
		*(--p_stk) = (CPU_STK)0x13131313uL; /* a3                                                   */
		*(--p_stk) = (CPU_STK)0x12121212uL; /* a2                                                   */
											/* Function Arguments/return values                     */
		*(--p_stk) = (CPU_STK)0x11111111uL; /* a1                                                   */
		*(--p_stk) = (CPU_STK)p_arg;		/* a0                                                   */
		*(--p_stk) = (CPU_STK)0x09090909uL; /* s1   : Saved register                                */
		*(--p_stk) = (CPU_STK)0x08080808uL; /* s0/fp: Saved register/Frame pointer                  */
											/* Temporary registers                                  */
		*(--p_stk) = (CPU_STK)0x07070707uL; /* t2                                                   */
		*(--p_stk) = (CPU_STK)0x06060606uL; /* t1                                                   */
		*(--p_stk) = (CPU_STK)0x05050505uL; /* t0                                                   */

		*(--p_stk) = (CPU_STK)0x04040404uL;	 /* tp: Thread pointer                                   */
		*(--p_stk) = (CPU_STK)0x03030303uL;	 /* gp: Global pointer                                   */
		*(--p_stk) = (CPU_STK)0x02020202uL;	 /* sp: Stack  pointer                                   */
		*(--p_stk) = (CPU_STK)OS_TaskReturn; /* ra: return address                                   */

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
	*                                          SYS TICK HANDLER
	*
	* Description: Handle the system tick (SysTick) interrupt, which is used to generate the uC/OS-III tick
	*              interrupt.
	*
	* Arguments  : None.
	*
	* Note(s)    : This function is defined with weak linking in 'riscv_hal_stubs.c' so that it can be
	*              overridden by the kernel port with same prototype
	*********************************************************************************************************
	*/

	void SysTick_Handler(void)
	{
		CPU_SR_ALLOC(); /* Allocate storage for CPU status register             */

		CPU_CRITICAL_ENTER();
		OSIntEnter(); /* Tell uC/OS-III that we are starting an ISR           */
		CPU_CRITICAL_EXIT();

		OSTimeTick(); /* Call uC/OS-III's OSTimeTick()                        */

		OSIntExit(); /* Tell uC/OS-III that we are leaving the ISR           */
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
	 void riscv_irq_handler(void)
	{
		HANDLER_PROLOGUE();
		uint_xlen_t this_cause = csr_read_mcause();
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
				csr_set_bits_mie(MIE_MTI_BIT_MASK);
				SysTick_Handler();
				break;
			case RISCV_INT_MSI_CODE:
				_SWITCH_CONTEXT();
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
		/*clean up */
		csr_write_mcause(0);
		HANDLER_EPILOGUE();
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
