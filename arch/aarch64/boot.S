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

    ldr     x0, =vectors // Charge l''adresse de ta table
    
    // Charge l''adresse de la table des vecteurs pour EL1
3:  ldr     x0, =vectors 
    msr     vbar_el1, x0

    // 1. HCR_EL2 : Prépare l''EL1 à s''exécuter en 64 bits (AArch64) en mettant le bit RW (31) à 1      
    ldr     x0, =(1 << 31)
    msr     hcr_el2, x0

    // 2. SPSR_EL2 : Configure l''état cible (EL1h = 0b0101) et masque les interruptions (0x3C0)        
    // 0x3C5 = DAIF masqués (1111) + M[4:0] configuré pour EL1h (00101)
    ldr     x0, =0x3C5
    msr     spsr_el2, x0

    // 3. ELR_EL2 : Configure l''adresse vers laquelle sauter (kernel_main)
    ldr     x0, =kernel_main
    msr     elr_el2, x0

    // 4. SP_EL1 : Initialise le pointeur de pile pour l''EL1
    ldr     x0, =_start
    msr     sp_el1, x0

    // 5. Le grand saut : Fait croire au CPU qu''il revient d''une exception
    // Il va lire ELR_EL2, SPSR_EL2 et atterrir dans kernel_main en EL1 !
    eret
halt:
    wfe
    b halt