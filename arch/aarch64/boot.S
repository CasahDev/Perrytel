// AArch64 mode

// To keep this in the first portion of the binary.
.section ".text.boot"

// Make _start global.
.globl _start

// Entry point for the kernel. Registers:
// x0 -> 32 bit pointer to DTB in memory (primary core only) / 0 (secondary cores)
// x1 -> 0
// x2 -> 0
// x3 -> 0
// x4 -> 32 bit kernel entry point, _start location
_start:
    // read mpidr_el1, and get core id
    mrs     x0, mpidr_el1
    and     x0, x0, #3
    cbz     x0, 2f
    // cpu id > 0, stop
1:  wfe
    b       1b

2:  // set stack before our code
    ldr     x5, =_start
    mov     sp, x5

    // clear bss
    ldr     x5, =__bss_start
    ldr     w6, =__bss_size
2:  cbz     w6, 3f
    str     xzr, [x5], #8
    subs    w6, w6, #8
    b.gt    2b

    // jump to C code, should not return
3:  bl      kernel_main
    // for failsafe, halt this core
halt:
    wfe
    b halt