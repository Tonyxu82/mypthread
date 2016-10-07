#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__
#include <atomic_ops.h>
#include <assert.h>

void spinlock_lock(AO_TS_t *);
void spinlock_unlock(AO_TS_t *);

#endif  /* __SPINLOCK_H__ */
