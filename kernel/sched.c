#include <kernel/sched.h>
#include <kernel/memory.h>
#include <stddef.h>

#define MAX_TASKS 64

struct Task tasks[MAX_TASKS] = {};
int task_id = 0;
int task_count = 0;                                                                                     


void create_task(task_function *task_function) {
    if (task_count >= MAX_TASKS) return;

    void *stack = kalloc(4096);
    TrapFrame *frame = (uint8_t *) stack + ((struct block_meta *) stack)->size - sizeof(TrapFrame);

    memset(frame, 0, sizeof(TrapFrame));

    frame->elr_el1 = (uint64_t) task_function;

    frame->spsr_el1 = 0x3C5; 

    struct Task task = {
        task_count,
        SLEEPING,
        frame
    };
    tasks[task_count] = task;
    
    task_count++;
}

void *schedule(void *sp) {
    tasks[task_id].stack_pointer = sp;
    tasks[task_id].state = SLEEPING;

    task_id = (task_id + 1) % task_count;
    tasks[task_id].state = RUNNING;

    return tasks[task_id].stack_pointer;
}