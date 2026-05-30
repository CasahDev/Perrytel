.section .text
.align 11

.macro kernel_entry
    // Save registers (except sp) on the stack.
    sub sp, sp, #(16 * 17) // Allocate space for 16 pairs of registers + 1 for x30
    stp x0, x1, [sp, #16 * 0]
    stp x2, x3, [sp, #16 * 1]
    stp x4, x5, [sp, #16 * 2]
    stp x6, x7, [sp, #16 * 3]
    stp x8, x9, [sp, #16 * 4]
    stp x10, x11, [sp, #16 * 5]
    stp x12, x13, [sp, #16 * 6]
    stp x14, x15, [sp, #16 * 7]
    stp x16, x17, [sp, #16 * 8]
    stp x18, x19, [sp, #16 * 9]
    stp x20, x21, [sp, #16 * 10]
    stp x22, x23, [sp, #16 * 11]
    stp x24, x25, [sp, #16 * 12]
    stp x26, x27, [sp, #16 * 13]
    stp x28, x29, [sp, #16 * 14]
    str x30, [sp, #16 * 15]

    mrs x0, elr_el2 // Save the return address in x32
    mrs x1, spsr_el2 // Save the return address in x33
    stp x0, x1, [sp, #16 * 16] // Save the return address on the stack
.endm

.macro kernel_exit
    ldp x0, x1, [sp, #16 * 16] // Restore the return address from the stack
    msr elr_el2, x0 // Restore the return address to elr_el
    msr spsr_el2, x1 // Restore the return address to spsr_el1
    
    ldp x0, x1, [sp, #16 * 0]
    ldp x2, x3, [sp, #16 * 1]
    ldp x4, x5, [sp, #16 * 2]
    ldp x6, x7, [sp, #16 * 3]
    ldp x8, x9, [sp, #16 * 4]
    ldp x10, x11, [sp, #16 * 5]
    ldp x12, x13, [sp, #16 * 6]
    ldp x14, x15, [sp, #16 * 7]
    ldp x16, x17, [sp, #16 * 8]
    ldp x18, x19, [sp, #16 * 9]
    ldp x20, x21, [sp, #16 * 10]
    ldp x22, x23, [sp, #16 * 11]
    ldp x24, x25, [sp, #16 * 12]
    ldp x26, x27, [sp, #16 * 13]
    ldp x28, x29, [sp, #16 * 14]
    ldr x30, [sp, #16 * 15]
    add sp, sp, #(16 * 17)
    eret
.endm


.macro ventry label
    .align 7            // 128 byte alignment for the next entry
    kernel_entry
    b \label
.endm

.globl vectors
vectors:
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry irq_handler_el1h
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry
    ventry invalid_entry

// Default handlers for exceptions. These will just loop indefinitely, but you can replace them with something more useful if you want.
invalid_entry:
    b invalid_entry

irq_handler_el1h:
    b irq_handler_el1h