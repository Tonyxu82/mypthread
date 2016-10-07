/*************************************************
 *
 *  CS533 Project: User level thread
 *  File: scheduler.h
 *  Author:
 *        Lai Xu  929272440 (xulai@pdx.edu)
 *        Bo Chen 904039384 (chenbo@pdx.edu)
 *  Date: Oct.24.2014
 *  Desc: defining the control structure of the thread.
 *        declaring the function for start and switch the
 *        thread.
 *
*************************************************/
#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "queue.h"
#include "spinlock.h"

extern void*safe_mem(int, void*);
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)

extern int safe_printf(const char *fmt, ...);

struct thread_t;

extern struct queue* p_ready_queue;
extern AO_TS_t lock_ready_queue;

extern struct thread_t * get_current_thread();
extern void set_current_thread(struct thread_t*);
#define p_current_thread (get_current_thread())

/* Change the number below to specify how many kernel threads you want to have */
#define MAX_KERNEL_THREADS 15

/* The default size of the thread stack */
#define MTHREAD_STACK_SIZE 1024*1024
/*Initial thread id, reserve purpose*/
#define INIT_THREAD_ID  0

/*the state of the thread*/
typedef enum{
	RUNNING,
	READY,
	BLOCKED,
	DONE
}state_t;

/* The priority of the thread*/
typedef enum{
	PRI_HIGH,
	PRI_MEDIUM,
	PRI_NORMAL
}priority_t;


/* The structure for the entry of thread control table*/
typedef void (*init_func)(void *);

/* The thread control block */
typedef struct thread_t
{
	void*      p_stack;     //the pointer to the stack owned by the thread
	uint64_t   i_stacklen;  //the size(bytes) of the stack
	init_func  p_func;  	//the function pointer to the initial function
	void*      arg;         //the parameter for the function
	uint64_t   i_tid;       //the thread ID
	state_t    state;     	//the running state
	priority_t priority;  	//the priority
        void*      p_stack_base; //the pointer to when the thread stack is allocated.
}thread_t;

struct fork_wrapper_arg{
  init_func target;
  void *arg;
}for_wrapper_arg;


/*initialize the queues and main thread */
int scheduler_begin();

/*
  If all threads have finished their job, this function will realse the space
  for those thread and the queues.
*/
void scheduler_end();

/*print ready queue for debuging purpose*/
void scheduler_print();

/* create a new thread for executing funciton target with argument arg */
void thread_fork(void(*target)(void*), void * arg);

/* Doing something after a thread has finished its job */
void thread_finish();

/* Start a new thread */
int thread_start(thread_t *current_thread, thread_t *new_thread);

/* Switch between two thread*/
int thread_switch(thread_t *current_thread, thread_t *new_thread);

/* yield the processor to the other threads */
void yield();

/* asynchronous read */
ssize_t read_wrap(int fd, void* buf, size_t count);

#endif //__SCHEDULER_H__
