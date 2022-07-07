/*******************************************************************************
 * Copyright(c) 2021 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this software except as stipulated in the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

/*!
 * @file bf_sys_sem.c
 * @date
 *
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>

#include <target-sys/bf_sal/bf_sys_mem.h>
#include <target-sys/bf_sal/bf_sys_sem.h>

int bf_sys_mutex_init(bf_sys_mutex_t *mtx) {
  int x;
  pthread_mutex_t *m;
  pthread_mutexattr_t a;

  pthread_mutexattr_init(&a);
  pthread_mutexattr_settype(&a, PTHREAD_MUTEX_ERRORCHECK);

  m = (pthread_mutex_t *)bf_sys_malloc(sizeof(pthread_mutex_t));
  mtx->bf_mutex = m;
  if (!m) {
    pthread_mutexattr_destroy(&a);
    return -1;
  }

  x = pthread_mutex_init(m, &a);
  pthread_mutexattr_destroy(&a);
  return x;
}

int bf_sys_mutex_del(bf_sys_mutex_t *mtx) {
  int err;

  err = pthread_mutex_destroy((pthread_mutex_t *)(mtx->bf_mutex));
  if (err) {
    return err;
  } else {
    bf_sys_free(mtx->bf_mutex);
    mtx->bf_mutex = NULL;
    return 0;
  }
}

int bf_sys_mutex_lock(bf_sys_mutex_t *mtx) {
  pthread_mutex_t *m = (pthread_mutex_t *)(mtx->bf_mutex);

  return (pthread_mutex_lock(m));
}

int bf_sys_mutex_trylock(bf_sys_mutex_t *mtx) {
  pthread_mutex_t *m = (pthread_mutex_t *)(mtx->bf_mutex);

  return (pthread_mutex_trylock(m));
}

int bf_sys_mutex_timedlock(bf_sys_mutex_t *mtx, long abs_sec, long abs_nsec) {
  pthread_mutex_t *m = (pthread_mutex_t *)(mtx->bf_mutex);
  struct timespec tm;
  tm.tv_sec = abs_sec;
  tm.tv_nsec = abs_nsec;

  return (pthread_mutex_timedlock(m, &tm));
}

int bf_sys_mutex_unlock(bf_sys_mutex_t *mtx) {
  pthread_mutex_t *m = (pthread_mutex_t *)(mtx->bf_mutex);

  return (pthread_mutex_unlock(m));
}

/*
 * Recursive Mutext APIs.
 */
int bf_sys_rmutex_init(bf_sys_rmutex_t *mtx) {
  int x;
  pthread_mutex_t *m;
  pthread_mutexattr_t a;

  pthread_mutexattr_init(&a);
  pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);

  m = (pthread_mutex_t *)bf_sys_malloc(sizeof(pthread_mutex_t));
  if (!m) {
    pthread_mutexattr_destroy(&a);
    return -1;
  }
  *mtx = m;

  x = pthread_mutex_init(m, &a);
  pthread_mutexattr_destroy(&a);
  return x;
}

int bf_sys_rmutex_del(bf_sys_rmutex_t *mtx) {
  int err;
  pthread_mutex_t *m = (pthread_mutex_t *)(*mtx);

  err = pthread_mutex_destroy(m);
  if (err) {
    return err;
  } else {
    bf_sys_free(*mtx);
    *mtx = NULL;
    return 0;
  }
}

int bf_sys_rmutex_lock(bf_sys_rmutex_t *mtx) {
  pthread_mutex_t *m = (pthread_mutex_t *)(*mtx);
  return (pthread_mutex_lock(m));
}

int bf_sys_rmutex_trylock(bf_sys_rmutex_t *mtx) {
  pthread_mutex_t *m = (pthread_mutex_t *)(*mtx);
  return (pthread_mutex_trylock(m));
}

int bf_sys_rmutex_unlock(bf_sys_rmutex_t *mtx) {
  pthread_mutex_t *m = (pthread_mutex_t *)(*mtx);
  return (pthread_mutex_unlock(m));
}

/*
 * Conditional Variable APIs.
 */
int bf_sys_cond_init(bf_sys_cond_t *c) {
  int x;

  pthread_cond_t *cv = bf_sys_malloc(sizeof(pthread_cond_t));
  if (!cv)
    return -1;
  x = pthread_cond_init(cv, NULL);
  if (x) {
    bf_sys_free(cv);
    return -1;
  }
  *c = cv;
  return 0;
}
int bf_sys_cond_del(bf_sys_cond_t *c) {
  int x;

  pthread_cond_t *cv = *c;
  x = pthread_cond_destroy(cv);
  if (x)
    return -1;
  bf_sys_free(*c);
  *c = NULL;
  return 0;
}
int bf_sys_cond_wait(bf_sys_cond_t *c, bf_sys_mutex_t *m) {
  pthread_cond_t *cv = *c;
  pthread_mutex_t *mtx = m->bf_mutex;
  return pthread_cond_wait(cv, mtx);
}
int bf_sys_cond_wake(bf_sys_cond_t *c) {
  pthread_cond_t *cv = *c;
  return pthread_cond_signal(cv);
}
int bf_sys_cond_broadcast(bf_sys_cond_t *c) {
  pthread_cond_t *cv = *c;
  return pthread_cond_broadcast(cv);
}

/* unnamed semaphore APIS */
int bf_sys_sem_init(bf_sys_sem_t *sem, int shared, unsigned int initial) {
  int x;
  sem_t *semaphore = bf_sys_malloc(sizeof(sem_t));
  if (!semaphore)
    return -1;

  x = sem_init(semaphore, shared, initial);
  if (x) {
    bf_sys_free(semaphore);
    *sem = NULL;
    return -1;
  } else {
    *sem = semaphore;
    return 0;
  }
}

int bf_sys_sem_destroy(bf_sys_sem_t *sem) {
  int x;
  sem_t *s = *sem;

  x = sem_destroy(s);

  if (x) {
    return -1;
  } else {
    bf_sys_free(*sem);
    *sem = NULL;
    return 0;
  }
}

int bf_sys_sem_wait(bf_sys_sem_t *sem) {
  sem_t *s = *sem;
  return (sem_wait(s));
}

int bf_sys_sem_trywait(bf_sys_sem_t *sem) {
  sem_t *s = *sem;
  return (sem_trywait(s));
}

int bf_sys_sem_post(bf_sys_sem_t *sem) {
  sem_t *s = *sem;
  return (sem_post(s));
}

/**
 * rw lock APIs
 */
int bf_sys_rwlock_init(bf_sys_rwlock_t *lock, void *rdlock_attr) {
  pthread_rwlock_t *rw_lock;
  pthread_rwlockattr_t *attr = (pthread_rwlockattr_t *)rdlock_attr;

  rw_lock = (pthread_rwlock_t *)bf_sys_malloc(sizeof(pthread_rwlock_t));
  lock->bf_rwlock = rw_lock;
  if (!rw_lock) {
    return -1;
  }

  return (pthread_rwlock_init(rw_lock, attr));
}

int bf_sys_rwlock_del(bf_sys_rwlock_t *lock) {
  int err;

  err = pthread_rwlock_destroy((pthread_rwlock_t *)(lock->bf_rwlock));
  if (err) {
    return err;
  } else {
    bf_sys_free(lock->bf_rwlock);
    return 0;
  }
}

int bf_sys_rwlock_rdlock(bf_sys_rwlock_t *lock) {
  return (pthread_rwlock_rdlock((pthread_rwlock_t *)(lock->bf_rwlock)));
}

int bf_sys_rwlock_tryrdlock(bf_sys_rwlock_t *lock) {
  return (pthread_rwlock_tryrdlock((pthread_rwlock_t *)(lock->bf_rwlock)));
}

int bf_sys_rwlock_timedrdlock(bf_sys_rwlock_t *lock, long abs_sec,
                              long abs_nsec) {
  struct timespec tm;
  tm.tv_sec = abs_sec;
  tm.tv_nsec = abs_nsec;

  return (
      pthread_rwlock_timedrdlock((pthread_rwlock_t *)(lock->bf_rwlock), &tm));
}

int bf_sys_rwlock_wrlock(bf_sys_rwlock_t *lock) {
  return (pthread_rwlock_wrlock((pthread_rwlock_t *)(lock->bf_rwlock)));
}

int bf_sys_rwlock_trywrlock(bf_sys_rwlock_t *lock) {
  return (pthread_rwlock_trywrlock((pthread_rwlock_t *)(lock->bf_rwlock)));
}

int bf_sys_rwlock_timedwrlock(bf_sys_rwlock_t *lock, long abs_sec,
                              long abs_nsec) {
  struct timespec tm;
  tm.tv_sec = abs_sec;
  tm.tv_nsec = abs_nsec;

  return (
      pthread_rwlock_timedwrlock((pthread_rwlock_t *)(lock->bf_rwlock), &tm));
}

int bf_sys_rwlock_unlock(bf_sys_rwlock_t *lock) {
  return (pthread_rwlock_unlock((pthread_rwlock_t *)(lock->bf_rwlock)));
}

/*
 * rw_mutex_lock_apis
 */
int bf_sys_rw_mutex_lock_init(bf_sys_rw_mutex_lock_t *rwlock) {
  int status, status1, status2;
  if (rwlock == NULL)
    return EINVAL;

  if (rwlock->valid == 1)
    return EBUSY;

  rwlock->r_active = 0;
  rwlock->r_wait = 0;
  rwlock->w_active = 0;
  rwlock->w_wait = 0;

  rwlock->mutex = (pthread_mutex_t *)bf_sys_malloc(sizeof(pthread_mutex_t));
  if (rwlock->mutex == NULL)
    return -1;

  status = pthread_mutex_init(rwlock->mutex, NULL);
  if (status != 0) {
    bf_sys_free(rwlock->mutex);
    return status;
  }

  rwlock->read = (pthread_cond_t *)bf_sys_malloc(sizeof(pthread_cond_t));
  if (rwlock->read == NULL) {
    status = pthread_mutex_destroy(rwlock->mutex);
    bf_sys_free(rwlock->mutex);
    return (status != 0 ? status : -1);
  }
  status = pthread_cond_init(rwlock->read, NULL);
  if (status != 0) {
    /* Destroy mutex if creation of cond variable fails*/
    status1 = pthread_mutex_destroy(rwlock->mutex);
    bf_sys_free(rwlock->mutex);
    bf_sys_free(rwlock->read);
    return (status1 != 0 ? status1 : status);
  }

  rwlock->write = (pthread_cond_t *)bf_sys_malloc(sizeof(pthread_cond_t));
  if (rwlock->write == NULL) {
    status = pthread_mutex_destroy(rwlock->mutex);
    bf_sys_free(rwlock->mutex);
    status1 = pthread_cond_destroy(rwlock->read);
    bf_sys_free(rwlock->read);
    return (status != 0 ? status : (status1 != 0 ? status1 : -1));
  }

  status = pthread_cond_init(rwlock->write, NULL);
  if (status != 0) {
    /* Destroy mutex and read cond variable if creation of write cond variable
     * fails*/
    status1 = pthread_mutex_destroy(rwlock->mutex);
    status2 = pthread_cond_destroy(rwlock->read);
    bf_sys_free(rwlock->mutex);
    bf_sys_free(rwlock->read);
    bf_sys_free(rwlock->write);
    return (status1 != 0 ? status1 : (status2 != 0 ? status2 : status));
  }

  rwlock->valid = 1;
  return 0;
}

int bf_sys_rw_mutex_lock_del(bf_sys_rw_mutex_lock_t *rwlock) {
  int status, status1, status2;
  if (rwlock == NULL)
    return EINVAL;

  if (rwlock->valid != 1)
    return EINVAL;

  status = pthread_mutex_lock(rwlock->mutex);
  if (status != 0)
    return status;

  // Return Busy if any threads are active
  if (rwlock->r_active > 0 || rwlock->w_active > 0) {
    status = pthread_mutex_unlock(rwlock->mutex);
    return (status != 0 ? status : EBUSY);
  }

  // Return Busy if any threads are waiting
  if (rwlock->r_wait > 0 || rwlock->w_wait > 0) {
    status = pthread_mutex_unlock(rwlock->mutex);
    return (status != 0 ? status : EBUSY);
  }

  rwlock->valid = 0;
  status = pthread_mutex_unlock(rwlock->mutex);
  if (status != 0)
    return status;

  // Destroy the mutex and Condition variables and free the memory
  status = pthread_mutex_destroy(rwlock->mutex);
  bf_sys_free(rwlock->mutex);
  status1 = pthread_cond_destroy(rwlock->read);
  bf_sys_free(rwlock->read);
  status2 = pthread_cond_destroy(rwlock->write);
  bf_sys_free(rwlock->write);

  return (status != 0 ? status : (status1 != 0 ? status1 : status2));
}

int bf_sys_rw_mutex_lock_rdlock(bf_sys_rw_mutex_lock_t *rwlock) {
  int status = 0, status1 = 0;
  if (rwlock == NULL)
    return EINVAL;

  if (rwlock->valid != 1)
    return EINVAL;

  status = pthread_mutex_lock(rwlock->mutex);
  if (status != 0)
    return status;

  // Wait until all the write threads are finished if any
  if (rwlock->w_active || rwlock->w_wait) {
    rwlock->r_wait++;
    while (rwlock->w_active || rwlock->w_wait) {
      status = pthread_cond_wait(rwlock->read, rwlock->mutex);
      if (status != 0)
        break;
    }
    rwlock->r_wait--;
  }

  if (status == 0)
    rwlock->r_active++;

  status1 = pthread_mutex_unlock(rwlock->mutex);
  return (status1 != 0 ? status1 : status);
}

int bf_sys_rw_mutex_lock_wrlock(bf_sys_rw_mutex_lock_t *rwlock) {
  int status = 0, status1 = 0;
  if (rwlock == NULL)
    return EINVAL;
  if (rwlock->valid != 1)
    return EINVAL;

  status = pthread_mutex_lock(rwlock->mutex);
  if (status != 0)
    return status;

  // Wait until the active write or read threads are finished
  if (rwlock->w_active || rwlock->r_active) {
    rwlock->w_wait++;
    while (rwlock->w_active || rwlock->r_active) {
      status = pthread_cond_wait(rwlock->write, rwlock->mutex);
      if (status != 0)
        break;
    }
    rwlock->w_wait--;
  }

  if (status == 0)
    rwlock->w_active++;

  status1 = pthread_mutex_unlock(rwlock->mutex);
  return (status1 != 0 ? status1 : status);
}

int bf_sys_rw_mutex_lock_rdunlock(bf_sys_rw_mutex_lock_t *rwlock) {
  int status = 0, status2 = 0;
  if (rwlock == NULL)
    return EINVAL;

  if (rwlock->valid != 1)
    return EINVAL;

  status = pthread_mutex_lock(rwlock->mutex);
  if (status != 0)
    return status;

  rwlock->r_active--;

  // Signal write if no actuve read threads.
  if (rwlock->r_active == 0)
    if (rwlock->w_wait > 0)
      status2 = pthread_cond_signal(rwlock->write);

  status = pthread_mutex_unlock(rwlock->mutex);
  return (status != 0 ? status : status2);
}

int bf_sys_rw_mutex_lock_wrunlock(bf_sys_rw_mutex_lock_t *rwlock) {
  int status = 0, status2 = 0;
  if (rwlock == NULL)
    return EINVAL;

  if (rwlock->valid != 1)
    return EINVAL;

  status = pthread_mutex_lock(rwlock->mutex);
  if (status != 0)
    return status;

  // Signal write if thread are waiting to write
  rwlock->w_active = 0;
  if (rwlock->w_wait > 0)
    status2 = pthread_cond_signal(rwlock->write);

  // Else signal read if threads are waiting
  else if (rwlock->r_wait > 0)
    status2 = pthread_cond_broadcast(rwlock->read);

  status = pthread_mutex_unlock(rwlock->mutex);
  return (status != 0 ? status : status2);
}

int bf_sys_compare_and_swap(bf_sys_cmp_and_swp_t *var,
                            bf_sys_cmp_and_swp_t old_val,
                            bf_sys_cmp_and_swp_t new_val) {
  return __sync_bool_compare_and_swap(var, old_val, new_val);
}
