.section .text
.align 11

.macro ventry label
    b \label
    .align 7            // 128 byte alignment for the next entry
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