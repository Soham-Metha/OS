/* scheduler.h */
#ifndef SCHEDULER_1
#define SCHEDULER_1
#include <common/types.h>

typedef void (*func)(void);

typedef enum Task_State {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_EXITED,
} Task_State;

typedef struct Task {
    uint32 t_id;
    Task_State t_state;
    func entry;
    struct Task* next;
} Task;

void schedule();
void scheduler_init(func init_main);
void reschedule(Task_State t);
void create_task(func f);
void resume(Task_State ts);

#endif
#ifdef IMPL_SCHEDULER_1
#undef IMPL_SCHEDULER_1

#include <common/heap.h>

Task* current   = (Task*)0;
Task* run_queue = (Task*)0;

uint64 next_pid = 0;

private
Task* pick_next_runnable()
{
    for (Task* t = current->next; t; t = t->next) {
        if (t->t_state == TASK_READY) {
            return t;
        }
    }
    for (Task* t = run_queue; t; t = t->next) {
        if (t->t_state == TASK_READY) {
            return t;
        }
    }
    return (Task*)0;
}

void schedule()
{
    Task* next = pick_next_runnable();
    if (next == (Task*)0) {
        return;
    }
    if (next != current) {
        current          = next;
        current->t_state = TASK_RUNNING;
        current->entry();
        if (current->t_state != TASK_WAITING)
            current->t_state = TASK_EXITED;
    }
}

private
void enque_runque(Task* t)
{
    if (!run_queue) {
        run_queue = t;
    } else {
        Task* tmp = run_queue;
        while (tmp->next) {
            tmp = tmp->next;
        }
        tmp->next = t;
    }
}

void create_task(func f)
{
    Task* t    = (Task*)kmalloc(sizeof(Task));
    t->t_id    = next_pid++;
    t->t_state = TASK_READY;
    t->entry   = f;
    t->next    = (Task*)0;

    enque_runque(t);
}

void scheduler_init(func init_main)
{
    create_task(init_main);
    schedule();
}

void reschedule(Task_State t)
{
    current->t_state = t;
    schedule();
}

void resume(Task_State ts)
{
    for (Task* t = run_queue; t; t = t->next) {
        if (t->t_state == ts) {
            t->t_state = TASK_READY;
        }
    }
}

#endif
