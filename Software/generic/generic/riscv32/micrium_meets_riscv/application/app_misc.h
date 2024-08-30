/*
 * misc.h
 *
 *  Created on: Jun 16, 2022
 *      Author: franc
 */

#ifndef __APPLICATION_MISC_H__
#define __APPLICATION_MISC_H__
#include "app_cfg.h"
#include "includes.h"

#if (APP_TASK_MATH>0u)
void APP_Math_Init(OS_ERR *err);
void APP_Task_math(void *p_arg);
#endif

#endif /* __APPLICATION_MISC_H__ */
