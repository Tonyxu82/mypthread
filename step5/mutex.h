#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "spinlock.h"
#include "queue.h"
#include "scheduler.h"

struct mutex{
  struct thread_t *holding_thread;
  struct queue waiting_threads;
  AO_TS_t spinlock_mutex;       /* spinlock for this mutex */
};

void mutex_init(struct mutex *);
void mutex_lock(struct mutex *);
void mutex_unlock(struct mutex *);

#endif  /* __MUTEX_H__ */
