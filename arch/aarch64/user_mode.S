.globl switch_to_user_mode
switch_to_user_mode:
    msr SP_EL0, x1
    msr ELR_EL1, x0
    mov x2, 0x0000
    msr SPSR_EL1, x2
    eret
