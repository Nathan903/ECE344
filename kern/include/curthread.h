#ifndef _CURTHREAD_H_
#define _CURTHREAD_H_

/*
 * The current thread.
 *
 * This is in its own header file (instead of thread.h) to reduce the
 * number of things that get recompiled when you change thread.h.
 */

struct thread;

extern struct thread *curthread;
extern int cur_max_pid;
#define MAX_PID 65536
struct pid_to_threadptr_node {
    pid_t pid;
    struct thread *parent_thread; 
    int exit_code;
    char has_exited;
};
#define PID_TABLE_LEN 2000 
extern struct pid_to_threadptr_node pid_to_threadptr[PID_TABLE_LEN];

#endif /* _CURTHREAD_H_ */
