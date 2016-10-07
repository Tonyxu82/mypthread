/*************************************************
 *
 *  CS533 Project: User level thread
 *  File: scheduler.c
 *  Author:
 *        Lai Xu  929272440 (xulai@pdx.edu)
 *        Bo Chen 904039384 (chenbo@pdx.edu)
 *  Date: Oct.24.2014
 *  Desc: defining the control structure of the thread.
 *        declaring the function for start and switch the
 *        thread.
 *
*************************************************/
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdarg.h>
#include "scheduler.h"
#include "queue.h"

/*the queue for threads in ready state*/
struct queue* p_ready_queue = NULL;
AO_TS_t lock_ready_queue = AO_TS_INITIALIZER;

#undef malloc
#undef free
void * safe_mem(int op, void * arg) {
  static AO_TS_t spinlock = AO_TS_INITIALIZER;
  void * result = 0;

  spinlock_lock(&spinlock);
  if(op == 0) {
    result = malloc((size_t)arg);
  } else {
    free(arg);
  }
  spinlock_unlock(&spinlock);
  return result;
}
#define malloc(arg) safe_mem(0, ((void*)(arg)))
#define free(arg) safe_mem(1, arg)

int safe_printf(const char * fmt, ...) {
  static AO_TS_t printf_spinlock = AO_TS_INITIALIZER;
  int result = 0;

  spinlock_lock(&printf_spinlock);

  va_list args;
  va_start(args, fmt);
  result = vprintf(fmt, args);
  va_end(args);

  spinlock_unlock(&printf_spinlock);

  return result;
}

static int kernel_thread_begin(void *arg) {
  /* initialize current thread in the new kernel thread*/
  thread_t* temp_thread = (thread_t*)malloc(sizeof(thread_t));
  assert(temp_thread && "memory allocation for p_current_thread failed.\n");
  memset(temp_thread, 0, sizeof(thread_t));
  temp_thread->state = RUNNING;

  set_current_thread(temp_thread);

  /* Enter the infinite loop of idle function */
  while(1){
    yield();
  }

  return 0;
}

/*initialize the queues and main thread */
int scheduler_begin()
{
  /* initialize ready queue */
  p_ready_queue = (struct queue*)malloc(sizeof(struct queue));
  assert(p_ready_queue && "memory allocation for p_ready_queue failed.\n");
  memset(p_ready_queue, 0, sizeof(struct queue));

  /* initialize current thread for main kernel thread */
  thread_t* temp_thread = (thread_t*)malloc(sizeof(thread_t));
  assert(temp_thread && "memory allocation for p_current_thread failed.\n");
  memset(temp_thread, 0, sizeof(thread_t));
  temp_thread->state = RUNNING;

  set_current_thread(temp_thread);


  int i;
  for(i = 1; i < MAX_KERNEL_THREADS; ++i) {
    char* p_stack = malloc(MTHREAD_STACK_SIZE);
    char* p_stack_top = p_stack + MTHREAD_STACK_SIZE;
    pid_t pid;
    pid = clone(kernel_thread_begin, p_stack_top,
                CLONE_THREAD | CLONE_VM | CLONE_SIGHAND | CLONE_FILES | CLONE_FS | CLONE_IO,
                NULL);
  }

  return 0;
}

void scheduler_end()
{
  while(1){
    spinlock_lock(&lock_ready_queue);

    if(!is_empty(p_ready_queue)) {
      spinlock_unlock(&lock_ready_queue);
      yield();
    } else {
      spinlock_unlock(&lock_ready_queue);
      break;
    }
  }
}

static void thread_fork_wrapper(void *arg){
  spinlock_unlock(&lock_ready_queue);

  struct fork_wrapper_arg* wrapper_args =
    (struct fork_wrapper_arg*) arg;

  wrapper_args->target(wrapper_args->arg);
}

/* create a new thread for executing funciton target with argument arg */
void thread_fork(void(*target)(void*), void * arg)
{
        /* allocate and initialize new thread table entry */
	thread_t* p_new_thread = (thread_t*)malloc(sizeof(thread_t));
        assert(p_new_thread && "Memory allocation for p_new_thread failed in thread_fork.\n");

	char* p_temp = malloc(MTHREAD_STACK_SIZE);
        assert(p_temp && "Memory allocation for p_temp failed in thread_fork.\n");

        /* prepare the input for thread_fork_wrapper */
        struct fork_wrapper_arg wrapper_args;
        wrapper_args.target = target;
        wrapper_args.arg = arg;

	//in x86-64 arch, the stack grows from high to low addresss
	p_new_thread->p_stack = (void*)(p_temp + MTHREAD_STACK_SIZE);
	p_new_thread->i_stacklen = MTHREAD_STACK_SIZE;
	p_new_thread->p_func = thread_fork_wrapper;
	p_new_thread->arg = (void *)(&wrapper_args);
	p_new_thread->state = RUNNING;

        assert(p_current_thread && "Fork while current thread is null.\n");

        spinlock_lock(&lock_ready_queue);

        /* change the state of current thread and put it in queue */
	p_current_thread->state = READY;
	thread_enqueue(p_ready_queue, p_current_thread);
	thread_t* p_old_thread = p_current_thread;
        set_current_thread(p_new_thread);

	thread_start(p_old_thread, p_current_thread);

        spinlock_unlock(&lock_ready_queue);
}

/* after a thread has finished its job */
void thread_finish()
{
  p_current_thread->state = DONE;
  yield();
}

/* yield the processor to the other threads */
void yield()
{
  spinlock_lock(&lock_ready_queue);

  if(is_empty(p_ready_queue)){
    spinlock_unlock(&lock_ready_queue);
    return;
  }

  if(p_current_thread->state == RUNNING) {
    p_current_thread->state = READY;
    thread_enqueue(p_ready_queue, p_current_thread);
  }

  struct thread_t *next_thread = thread_dequeue(p_ready_queue);

  assert(next_thread && "next_thread is null.\n");

  next_thread->state = RUNNING;

  struct thread_t *temp_thread = p_current_thread;
  set_current_thread(next_thread);

  thread_switch(temp_thread, p_current_thread);

  spinlock_unlock(&lock_ready_queue);
}
