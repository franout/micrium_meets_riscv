/*
 *********************************************************************************************************
 *
 *                                    MICRIUM BOARD SUPPORT PACKAGE
 *
 *                                          For Comet Simulator
 *
 *
 * Filename      : bsp.c
 * Version       : V1.00
 * Programmer(s) : franout (francesco.angione@polito.it)
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                             INCLUDE FILES
 *********************************************************************************************************
 */

#ifndef BSP_MODULE
#define BSP_MODULE
// RISC-V CSR definitions and access classes
#include "riscv-csr.h"
#include "riscv-interrupts.h"
#include "timer.h"

/*
 *********************************************************************************************************
 *                                      REGISTER & BIT FIELD DEFINES
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                       EXTERN GLOBAL VARIABLES
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                           LOCAL DEFINES
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                          LOCAL DATA TYPES
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                            LOCAL TABLES
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                       LOCAL GLOBAL VARIABLES
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *                                      LOCAL FUNCTION PROTOTYPES
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *********************************************************************************************************
 **                                        GLOBAL FUNCTIONS
 *********************************************************************************************************
 *********************************************************************************************************
 */

void BSP_Init(void)
{
  /*placeholder*/
  return;
}

void BSP_IntInit(void)
{
  // Global interrupt disable
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK );
  
  // Setup the IRQ handler entry point
  csr_write_mtvec((uint_xlen_t)riscv_irq_handler);

  // Global interrupt enable
  csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
  return;
}
/*
 *********************************************************************************************************
 *                                          BSP_OS_Tick_Init()
 *
 * Description : Initialize The SysTick Timer.
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.

 * Caller(s)   : Application.
 *
 * Note(s)     : none.
 *********************************************************************************************************
 */

void BSP_OS_Tick_Init()
{
  /*disable interrupt*/
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
  csr_write_mie(0);
  /*setup */
  mtimer_set_raw_time_cmp(MTIMER_SECONDS_TO_CLOCKS(OS_CFG_TICK_RATE_HZ));
  // Global interrupt enable
  csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}

/*
 *********************************************************************************************************
 *                                          BSP_OS_TickEnable()
 *
 * Description : enable The SysTick Timer.
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.o

 * Caller(s)   : Application.
 *
 * Note(s)     : none.
 *********************************************************************************************************
 */
void BSP_OS_TickEnable(void)
{
  /*disable interrupt*/
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);

  // Enable MIE.MTI
  csr_set_bits_mie(MIE_MTI_BIT_MASK);

  // Global interrupt enable
  csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}

/*
 *********************************************************************************************************
 *                                          BSP_OS_TickDisable()
 *
 * Description : Disable The SysTick Timer.
 *
 * Argument(s) : none.
 *
 * Return(s)   : none.

 * Caller(s)   : Application.
 *
 * Note(s)     : none.
 *********************************************************************************************************
 */
void BSP_OS_TickDisable(void)
{
  /*disable interrupt*/
  csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
  csr_write_mie(0);

  // disable MIE.MTI
  csr_clr_bits_mie(MIE_MTI_BIT_MASK);

  // Global interrupt enable
  csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
}

#endif
