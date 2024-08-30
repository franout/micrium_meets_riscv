/*					\\\ Sapere Aude ///
 *  app_cfg.h
 *  Created on: 17 Dec 2020
 *  Description: 
 *  Author : (Franout) Angione Francesco francesco.angione@polito.it  
 * -----------------------------------------------------------------------------
 *  Copyright (c) 2014-2020 All rights reserved 
 * -----------------------------------------------------------------------------
 */
#ifndef __APP_CFG_H_
#define __APP_CFG_H_

/*
 *********************************************************************************************************
 *                                       Aditional application task
 *********************************************************************************************************
 */


#define		APP_TASK_MATH						1u

/*
 *********************************************************************************************************
 *                                       ADDITIONAL uC/MODULE ENABLES
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                            TASK PRIORITIES
 *********************************************************************************************************
 */

#ifdef APP_CFG_TASK_START_PRIO
#if (OS_CFG_TICK_TASK_PRIO >= APP_CFG_TASK_START_PRIO)
#error TICK TASK PRIORITY IS LOWER THAN APP TASK PRIORITY
#endif
#else
#define APP_CFG_TASK_START_PRIO  				OS_CFG_TICK_TASK_PRIO+1
#define APP_CFG_TASK_TRY_PRIO					APP_CFG_TASK_START_PRIO+1
#endif
#define  OS_TASK_TMR_PRIO                         (OS_CFG_LOWEST_PRIO - 2)

#if (CLK_CFG_EXT_EN==DEF_DISABLED) /*calendar is software maintained*/
#define CLK_OS_CFG_TASK_PRIO 						12u
#endif



#define APP_CFG_TASK_MISC_PRIO						14u
#define  APP_CFG_TASK_MATH_PRIO						50u

/*
 *********************************************************************************************************
 *                                            TASK STACK SIZES
 *********************************************************************************************************
 */

#define APP_CFG_TASK_START_STK_SIZE					256u

#define MATH_OPS_LENGTH 							256u /*size of data structure in math task*/
#define APP_CFG_TASK_MATH_STK_SIZE					(MATH_OPS_LENGTH*4)+512u

/*
 *********************************************************************************************************
 *                                     TRACE / DEBUG CONFIGURATION
 *********************************************************************************************************
 */

#define  TRACE_LEVEL_OFF                                  0u
#define  TRACE_LEVEL_INFO                                 1u
#define  TRACE_LEVEL_DBG                                  2u

#define  APP_CFG_TRACE_LEVEL                    TRACE_LEVEL_DBG
#define  APP_CFG_TRACE                          printf

#define  APP_TRACE_INFO(x)                     ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_CFG_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                      ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_CFG_TRACE x) : (void)0)

/*
 *********************************************************************************************************
 *                                                LIBS
 *********************************************************************************************************
 */

#endif /* __APP_CFG_H_ */
