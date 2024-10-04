// RISC-V CSR definitions and access classes
#include "riscv-csr.h"
#include "riscv-interrupts.h"
#include "timer.h"


void CPU_IntDis(void){
    // Global interrupt disable (all exceptt the timer)
    //csr_clr_bits_mstatus(MSTATUS_MIE_BIT_MASK);
    //csr_write_mie(0);
    csr_clr_bits_mie(MIE_MEI_BIT_MASK);
}

void    CPU_IntEn     (void){
// Global interrupt enable 
    //csr_set_bits_mstatus(MSTATUS_MIE_BIT_MASK);
    csr_set_bits_mie(MIE_MEI_BIT_MASK|MIE_MSI_BIT_MASK|MIE_MTI_BIT_MASK);
}     


    

    