#include "mutex.h"
#include <assert.h>

void mutex_init(struct mutex* p_mutex)
{
  p_mutex->holding_thread = NULL;
  p_mutex->waiting_threads.head = NULL;
  p_mutex->waiting_threads.tail = NULL;
  p_mutex->spinlock_mutex = AO_TS_INITIALIZER;
}

static block(AO_TS_t *spinlock_mutex)
{
  assert(p_current_thread->state == BLOCKED);

  /* acquire the lock of ready queue */
  spinlock_lock(&lock_ready_queue);
  /* unlock the lock of mutex */
  spinlock_unlock(spinlock_mutex);

  assert(!is_empty(p_ready_queue) && "p_ready_queue is empty.\n");

  struct thread_t *next_thread = thread_dequeue(p_ready_queue);

  assert(next_thread && "next_thread is null.\n");

  next_thread->state = RUNNING;

  struct thread_t *temp_thread = p_current_thread;
  set_current_thread(next_thread);

  thread_switch(temp_thread, p_current_thread);

  /* unlock the lock of ready queue */
  spinlock_unlock(&lock_ready_queue);
}

void mutex_lock(struct mutex *p_mutex)
{
  assert(p_mutex);
  assert(p_current_thread->state == RUNNING);

  spinlock_lock(&p_mutex->spinlock_mutex);

  /* If the mutex is avaiable, acquire the mutex */
  if(p_mutex->holding_thread == NULL) {
    p_mutex->holding_thread = p_current_thread;
    spinlock_unlock(&p_mutex->spinlock_mutex);
    return;
  }

  /* If the mutex is not avaiable, change the sate of current thread as BLOCK
   * and put it in the waiting queue of the current mutex
   */
  p_current_thread->state = BLOCKED;
  thread_enqueue(&p_mutex->waiting_threads, p_current_thread);
  block(&p_mutex->spinlock_mutex);
}

void mutex_unlock(struct mutex *p_mutex)
{
  spinlock_lock(&p_mutex->spinlock_mutex);

  assert(p_mutex->holding_thread == p_current_thread);
  p_mutex->holding_thread = NULL;

  /* wake up one of the threads that are waiting for this mutex by putting
   * the front thread in ready queue
   */
  struct thread_t *temp_thread;
  if(!is_empty(&p_mutex->waiting_threads)){
    temp_thread = thread_dequeue(&p_mutex->waiting_threads);

    spinlock_lock(&lock_ready_queue);

    temp_thread->state = READY;
    p_mutex->holding_thread = temp_thread;
    thread_enqueue(p_ready_queue, temp_thread);

    spinlock_unlock(&lock_ready_queue);
  }

  spinlock_unlock(&p_mutex->spinlock_mutex);
}
