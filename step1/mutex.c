#include "mutex.h"
#include <assert.h>

void mutex_init(struct mutex* p_mutex)
{
  p_mutex->holding_thread = NULL;
  p_mutex->waiting_threads.head = NULL;
  p_mutex->waiting_threads.tail = NULL;
}

void mutex_lock(struct mutex *p_mutex)
{
  assert(p_mutex);
  assert(p_current_thread->state == RUNNING);

  /* If the mutex is avaiable, acquire the mutex */
  if(p_mutex->holding_thread == NULL) {
    p_mutex->holding_thread = p_current_thread;
    return;
  }

  /* If the mutex is not avaiable, change the sate of current thread as BLOCK
   * and put it in the waiting queue of the current mutex
   */
  p_current_thread->state = BLOCKED;
  thread_enqueue(&p_mutex->waiting_threads, p_current_thread);
  yield();
}

void mutex_unlock(struct mutex *p_mutex)
{
  assert(p_mutex->holding_thread == p_current_thread);
  p_mutex->holding_thread = NULL;

  /* wake up one of the threads that are waiting for this mutex by putting
   * the front thread in ready queue
   */
  struct thread_t *temp_thread;
  if(!is_empty(&p_mutex->waiting_threads)){
    temp_thread = thread_dequeue(&p_mutex->waiting_threads);

    temp_thread->state = READY;
    p_mutex->holding_thread = temp_thread;
    thread_enqueue(p_ready_queue, temp_thread);
  }
}
