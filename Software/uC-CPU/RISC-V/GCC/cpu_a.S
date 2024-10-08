#********************************************************************************************************
#                                               uC/CPU
#                                    CPU CONFIGURATION & PORT LAYER
#
#                    Copyright 2004-2021 Silicon Laboratories Inc. www.silabs.com
#
#                                 SPDX-License-Identifier: APACHE-2.0
#
#               This software is subject to an open source license and is distributed by
#                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
#                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
#
#********************************************************************************************************

#********************************************************************************************************
#
#                                            CPU PORT FILE
#
#                                               RISC-V
#                                           GNU C Compiler
#
# Filename : cpu_a.S
# Version  : V1.32.01
#********************************************************************************************************


#********************************************************************************************************
#                                           PUBLIC FUNCTIONS
#********************************************************************************************************

    .global  CPU_SR_Save
    .global  CPU_SR_Restore
    .global  CPU_get_sp
    .global  CPU_get_gp



#********************************************************************************************************
#                                                EQUATES
#********************************************************************************************************

    .equ  CPU_MSTATUS_MIE,         0x8


#********************************************************************************************************
#                                      CODE GENERATION DIRECTIVES
#********************************************************************************************************

    .section .text



#********************************************************************************************************
#                                      CRITICAL SECTION FUNCTIONS
#
# Description : Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking, the
#               state of the interrupt disable flag is stored in the local variable 'cpu_sr' & interrupts
#               are then disabled ('cpu_sr' is allocated in all functions that need to disable interrupts).
#               The previous interrupt state is restored by copying 'cpu_sr' into the CPU's status register.
#
# Prototypes  : CPU_SR  CPU_SR_Save   (void);
#               void    CPU_SR_Restore(CPU_SR  cpu_sr);
#
# Note(s)     : (1) These functions are used in general like this :
#
#                       void  Task (void  *p_arg)
#                       {
#                           CPU_SR_ALLOC();                     /* Allocate storage for CPU status register */
#                               :
#                               :
#                           CPU_CRITICAL_ENTER();               /* cpu_sr = CPU_SR_Save()#                  */
#                               :
#                               :
#                           CPU_CRITICAL_EXIT();                /* CPU_SR_Restore(cpu_sr)#                  */
#                               :
#                       }
#********************************************************************************************************

CPU_SR_Save:
# Save the Machine status register
    csrr   a0, mstatus
# Disable global interupt
#    li     t0, CPU_MSTATUS_MIE
#    csrrc  zero, mstatus, t0
    ret


CPU_SR_Restore:
# restore the Machine status register previous state
    csrw   mstatus, a0
    ret

CPU_get_gp:
    mv a0, gp
    ret

CPU_get_sp:
    mv a0, sp
    ret

#********************************************************************************************************
#                                     CPU ASSEMBLY PORT FILE END
#********************************************************************************************************
