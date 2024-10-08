#********************************************************************************************************
#                                              uC/OS-III
#                                        The Real-Time Kernel
#
#                    Copyright 2009-2022 Silicon Laboratories Inc. www.silabs.com
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
#                                        ASSEMBLY LANGUAGE PORT
#                                              RISC-V PORT
#
# File      : os_cpu_a.S
# Version   : V3.08.02
#********************************************************************************************************
# For       : RISC-V RV32
# Toolchain : GNU C Compiler
#********************************************************************************************************
# Note(s)   : Hardware FP is not supported.
#********************************************************************************************************

#********************************************************************************************************
#                                          PUBLIC FUNCTIONS
#********************************************************************************************************

    .extern  OSRunning                               # External references
    .extern  OSPrioCur
    .extern  OSPrioHighRdy
    .extern  OSTCBCurPtr
    .extern  OSTCBHighRdyPtr
    .extern  OSIntExit
    .extern  OSTaskSwHook


    .global  OSStartHighRdy                          # Functions declared in this file
    .global  OSCtxSw
    .global  OSIntCtxSw


#********************************************************************************************************
#                                               EQUATES
#********************************************************************************************************


    .equ  RISCV_MSTATUS_MIE,         0x8

    .equ  RISCV_MIE_MSIE,            0x08            # M Soft Interrupt bit

    .equ  RISCV_MIE_MTIE,            0x80

    .equ  RISCV_MIP_MSIP,            0x08            # M Soft Interrupt bit


#********************************************************************************************************
#                                     CODE GENERATION DIRECTIVES
#********************************************************************************************************

.section .text


#********************************************************************************************************
#                                         START MULTITASKING
#                                      void OSStartHighRdy(void)
#
# Note(s) : 1) OSStartHighRdy() MUST:
#              a) Call OSTaskSwHook() then,
#              b) Set OSRunning to TRUE,
#              c) Set OSTCBHighRdyPtr->StkPtr = SP
#              d) Restore x1-x31; There is no need to restore x0 since it is always zero.
#              e) Enable interrupts (tasks will run with interrupts enabled).
#              f) Switch to highest priority task.
#********************************************************************************************************
.align 8
OSStartHighRdy:
# Disable interrupts
    li     t0, RISCV_MSTATUS_MIE
    csrrc  zero, mstatus, t0

# Execute OS task switch hook.
    jal    OSTaskSwHook

# OSRunning = TRUE;
    li     t0, 0x01
    la     t1, OSRunning
    sb     t0, 0(t1)

# SWITCH TO HIGHEST PRIORITY TASK
    la     t0, OSTCBHighRdyPtr
    lw     t1, 0(t0)
    lw     sp, 0(t1)
# Restore mstatus
    lw x30, 30 *4 (sp)
    csrrw x30, mstatus, x30
# Restore x1 to x31 registers
    lw x1,  0 * 4(sp)    
    lw x3,  1 * 4(sp)    
    lw x4,  2 * 4(sp)    
    lw x5,  3 * 4(sp)    
    lw x6,  4 * 4(sp)    
    lw x7,  5 * 4(sp)    
    lw x8,  6 * 4(sp)    
    lw x9,  7 * 4(sp)
    lw x10, 8 * 4(sp)    
    lw x11, 9 * 4(sp)    
    lw x12, 10 * 4(sp)    
    lw x13, 11 * 4(sp)    
    lw x14, 12 * 4(sp)    
    lw x15, 13 * 4(sp)    
    lw x16, 14 * 4(sp)    
    lw x17, 15 * 4(sp)    
    lw x18, 16 * 4(sp)    
    lw x19, 17 * 4(sp)    
    lw x20, 18 * 4(sp)    
    lw x21, 19 * 4(sp)    
    lw x22, 20 * 4(sp)    
    lw x23, 21 * 4(sp)    
    lw x24, 22 * 4(sp)    
    lw x25, 23 * 4(sp)    
    lw x26, 24 * 4(sp)    
    lw x27, 25 * 4(sp)    
    lw x28, 26 * 4(sp)    
    lw x29, 27 * 4(sp)    
    lw x30, 28 * 4(sp)    
    lw x31, 29 * 4(sp)    

# Use register x31 to jump to HIGHEST priority
    lw     x31, 31 * 4(sp)

# Compensate for the stack pointer
    addi   sp, sp, 32 * 4
# Clear pending interrupt for timer 
    
    li     t0, 0
    csrrs  zero, mie, t0
    # # for Comet simulator  
    li     t0, 0
    la t1, 0x50 # address of mip 
    sw t0,0(t1)
# Enable global interrupts
    li     t0, RISCV_MSTATUS_MIE
    csrrs  zero, mstatus, t0

    li     t0, RISCV_MIE_MSIE
    csrrs  zero, mie, t0
# Jump to HIGHEST priority task.
    jalr   x0, x31, 0


#********************************************************************************************************
#                       PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
#                   PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
#
# Note(s) : 1) OSCtxSw() is called when OS wants to perform a task context switch.  This function
#              triggers a synchronous software interrupt by writing into the MSIP register
#
#           2) OSIntCtxSw() is called by OSIntExit() when it determines a context switch is needed as
#              the result of an interrupt.  This function triggers a synchronous software interrupt by
#              writing into the MSIP register
#********************************************************************************************************
.align 8
OSCtxSw:
OSIntCtxSw:
# This will trigger a synchronous software interrupt; PRCI->MSIP[0] = 0x01;
    li     t0, RISCV_MIP_MSIP
    csrrs  zero, mip, t0 
    # # MIE_MSIE -- enable software interrupt bit
    li     t0, RISCV_MIE_MSIE
    csrrs  zero, mie, t0
    # # for Comet simulator  
    la t1, 0x50 # address of mip 
    sw t0,0(t1)
    ret

.align 8
#********************************************************************************************************
#                                             MODULE END
#*********************************************************************************************************
