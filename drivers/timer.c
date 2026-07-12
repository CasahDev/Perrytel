#include <stdint.h>
#include <drivers/timer.h>

volatile unsigned long time;

// Exemple pour lire la fréquence du timer
static inline uint64_t read_cntfrq(void) {
    uint64_t val;
    // On lit le registre système cntfrq_el0 et on le place dans 'val'
    asm volatile("mrs %0, cntfrq_el0" : "=r" (val));
    return val;
}

// Exemple pour écrire la valeur du compte à rebours
static inline void write_cntv_tval(uint32_t val) {
    // On prend 'val' et on l'écrit dans le registre cntv_tval_el0
    asm volatile("msr cntv_tval_el0, %0" : : "r" (val));
}

static inline void start_timer() {
    asm volatile("msr cntv_ctl_el0, %0;" : : "r" (1));
}

void timer_init(void) {
    uint64_t timer_value = read_cntfrq();
    write_cntv_tval(timer_value);
    start_timer();
}

void timer_reset() {
    write_cntv_tval(read_cntfrq());
}
void set_time(unsigned long new_time) {
    time = new_time; 
}

unsigned long get_time() {
    return time;
}