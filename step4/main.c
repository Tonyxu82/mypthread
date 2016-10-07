#include <stdio.h>
#include "scheduler.h"
#include "mutex.h"

/* Please make sure BUF_SIZE = USER_THREADS * LOOP_AMT */
#define BUF_SIZE 100000
#define USER_THREADS 500        /* Number of user level threads to fork */
#define LOOP_AMT 200

/* Comment the line below to test the shared data will be corruptted without the protection of mutex */
#define MUTEX_PROTECT_ENABLE

int shared_counter;
int shared_array[BUF_SIZE];

struct mutex mutex_shared_array;

AO_TS_t lock_printf = AO_TS_INITIALIZER;

void increment(void * arg) {
  int i;
  for(i = 0; i < LOOP_AMT; ++i) {
#ifdef MUTEX_PROTECT_ENABLE
    mutex_lock(&mutex_shared_array);
#endif
    shared_array[shared_counter] = shared_counter;
    shared_counter++;

#ifdef MUTEX_PROTECT_ENABLE
    mutex_unlock(&mutex_shared_array);
#endif

    yield();
  }

  *(int*)arg = 1;
}

int main(void)
{
  shared_counter = 0;
  memset(shared_array, 0, BUF_SIZE * sizeof(int));
  mutex_init(&mutex_shared_array);

  scheduler_begin();

  int finished_flags[USER_THREADS] = {0};
  int i;
  for(i = 0; i < USER_THREADS; ++i){
    thread_fork(increment, &finished_flags[i]);
  }

  scheduler_end();

  /* check whether the result is correct */
  int success = 1;
  for(i = 0; i < BUF_SIZE; ++i){
    if(shared_array[i] != i) {
      success = 0;
      break;
    }
  }

  spinlock_lock(&lock_printf);

  if(success) {
    safe_printf("\nsuccess!\n");
  } else {
    safe_printf("\nfailure!\n");
  }

  spinlock_unlock(&lock_printf);

}
