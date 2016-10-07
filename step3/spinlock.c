#include "spinlock.h"

void spinlock_lock(AO_TS_t *ts_var) {
  while(AO_test_and_set_acquire(ts_var) == AO_TS_SET);
}

void spinlock_unlock(AO_TS_t *ts_var) {
  assert( *ts_var == AO_TS_SET && "[Error] unlocking an unlocked lock.\n");

  AO_CLEAR(ts_var);
}
