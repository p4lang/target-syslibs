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
 * @file bf_sys_sem.h
 * @date
 *
 *
 */
#ifndef _BF_SYS_SEM_H_
#define _BF_SYS_SEM_H_

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup bf_sal-sem
 * @{
 */

typedef struct bf_sys_mutex_s {
  void *bf_mutex; /* OS abstracted context pointer */
} bf_sys_mutex_t;

typedef struct bf_sys_rwlock_s {
  void *bf_rwlock; /* OS abstracted context pointer */
} bf_sys_rwlock_t;

typedef struct bf_sys_rw_mutex_lock_s {
  void *mutex;  /* Mutex for the entire structure */
  void *read;   /* read condition variable*/
  void *write;  /* write condition variable*/
  int r_active; /* readers active */
  int w_active; /* writer active */
  int r_wait;   /* readers waiting */
  int w_wait;   /* writers waiting */
  int valid;    /* Set when valid */
} bf_sys_rw_mutex_lock_t;

typedef struct bf_sys_named_sem_s {
  void *bf_n_sem; /* OS abstracted context pointer */
} bf_sys_named_sem_t;

typedef void *bf_sys_sem_t;
typedef void *bf_sys_rmutex_t;
typedef void *bf_sys_cond_t;

typedef int bf_sys_cmp_and_swp_t;

/**
 * initialize a mutex
 * @param mutex
 *  pointer to mutex
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_mutex_init(bf_sys_mutex_t *mtx);

/**
 * destroy a mutex
 * @param mutex
 *  pointer to mutex
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_mutex_del(bf_sys_mutex_t *mtx);

/**
 * lock a mutex
 * @param mutex
 *  pointer to mutex
 * @return Status
 *  0 on Success(may block), implementation specific error on failure
 */
int bf_sys_mutex_lock(bf_sys_mutex_t *mtx);

/**
 * try locking a mutex or return immediately
 * @param mutex
 *  pointer to mutex
 * @return Status
 *  0 on Success(lock acquired), implementation specific error on failure
 *  (no wait)
 */
int bf_sys_mutex_trylock(bf_sys_mutex_t *mtx);

/**
 * lock a mutex while not not blocking for more than some period
 * @param mutex
 *  pointer to mutex
 * @param abs_sec
 *  absolute seconds for maximum block-period
 * @param abs_nsec
 *  absolute nano seconds (plus sec) for maximum block-period
 * @return Status
 *  0 on Success(lock acquired), implementation specific error on failure
 */
int bf_sys_mutex_timedlock(bf_sys_mutex_t *mtx, long abs_sec, long abs_nsec);

/**
 * unlock a mutex
 * @param mutex
 *  pointer to mutex
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_mutex_unlock(bf_sys_mutex_t *mtx);

int bf_sys_rmutex_init(bf_sys_rmutex_t *mtx);
int bf_sys_rmutex_lock(bf_sys_rmutex_t *mtx);
int bf_sys_rmutex_trylock(bf_sys_rmutex_t *mtx);
int bf_sys_rmutex_unlock(bf_sys_rmutex_t *mtx);
int bf_sys_rmutex_del(bf_sys_rmutex_t *mtx);

/* user space only */
/**
 * initialize a condition variable
 * @param c
 *  pointer to condition variable
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_cond_init(bf_sys_cond_t *c);
/**
 * destroy a condition variable
 * @param c
 *  pointer to condition variable
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_cond_del(bf_sys_cond_t *c);
/**
 * wait on a condition variable
 * @param c
 *  pointer to condition variable
 * @param m
 *  pointer to the condition's mutex
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_cond_wait(bf_sys_cond_t *c, bf_sys_mutex_t *m);
/**
 * wake up thread(s) waiting on the condition variable
 * @param c
 *  pointer to condition variable
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_cond_wake(bf_sys_cond_t *c);

/**
 * wake up all threads waiting on the condition variable
 * @param c
 *  pointer to condition variable
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_cond_broadcast(bf_sys_cond_t *c);

/**
 * create a semaphore
 * @param initial
 *  inital value of semaphore. 0: must be posted before being waited on
 *                            >0: subsequent wait would return immediately
 * @param shared
 *   1: shared, 0: exclusive use bycurrent process
 * @param sem
 *  pointer to  bf_sys_sem_t
 * @return int
 *  0 on Success, -1 on failure
 */
int bf_sys_sem_init(bf_sys_sem_t *sem, int shared, unsigned int initial);

/**
 * destroy a semaphore
 * @param sem
 *  pointer to semaphore
 * @return int
 *  0 on Success, -1 on failure
 */
int bf_sys_sem_destroy(bf_sys_sem_t *sem);

/**
 * decrement the semaphore, will block if it is already 0.
 * @param sem
 *  pointer to bf_sys_sem_t
 * @return int
 *  0 on Success, -1 on failure
 */
int bf_sys_sem_wait(bf_sys_sem_t *sem);

/**
 * try to decrement a semaphore without blocking
 * @param sem
 *  pointer to bf_sys_sem_t
 * @return int
 *  0 on Success, -1 on failure
 */
int bf_sys_sem_trywait(bf_sys_sem_t *sem);

/**
 * increment a semaphore
 * @param sem
 *  pointer to bf_sys_sem_t
 * @return int
 *  0 on Success, -1 on failure
 */
int bf_sys_sem_post(bf_sys_sem_t *sem);

/**
 * initialize a rdlock
 * @param lock
 *  pointer to rdlock
 * @param rw_lock_attr
 *  pointer to rwlock  attribute (OS/platform dependent)
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_rwlock_init(bf_sys_rwlock_t *lock, void *rw_lock_attr);

/**
 * destroy a rwlock
 * @param lock
 *  pointer to rdlock
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_rwlock_del(bf_sys_rwlock_t *lock);

/**
 * read lock a rw lock
 * @param lock
 *  pointer to bf_sys_rwlock_t
 * @return Status
 *  0 on Success(may block), implementation specific error on failure
 */
int bf_sys_rwlock_rdlock(bf_sys_rwlock_t *lock);

/**
 * read lock a rwlock while not not blocking for more than some period
 * @param lock
 *  pointer to rwlock
 * @param abs_sec
 *  absolute seconds for maximum block-period
 * @param abs_nsec
 *  absolute nano seconds (plus sec) for maximum block-period
 * @return Status
 *  0 on Success(rdlock acquired), -1 on failure
 */
int bf_sys_rwlock_timedrdlock(bf_sys_rwlock_t *lock, long abs_sec,
                              long abs_nsec);

/**
 * try to read lock a rw lock without blocking
 * @param sem
 *  pointer to bf_sys_rwlock_t
 * @return int
 *  0 on Success, -1 on failure
 */
int bf_sys_rwlock_tryrdlock(bf_sys_rwlock_t *lock);

/**
 * write lock a rw lock
 * @param lock
 *  pointer to bf_sys_rwlock_t
 * @return Status
 *  0 on Success(may block), implementation specific error on failure
 */
int bf_sys_rwlock_wrlock(bf_sys_rwlock_t *lock);

/**
 * write lock a rwlock while not not blocking for more than some period
 * @param lock
 *  pointer to rwlock
 * @param abs_sec
 *  absolute seconds for maximum block-period
 * @param abs_nsec
 *  absolute nano seconds (plus sec) for maximum block-period
 * @return Status
 *  0 on Success(wrlock acquired), -1 on failure
 */
int bf_sys_rwlock_timedwrlock(bf_sys_rwlock_t *lock, long abs_sec,
                              long abs_nsec);

/**
 * try to write lock a rw lock without blocking
 * @param sem
 *  pointer to bf_sys_rwlock_t
 * @return int
 *  0 on Success, -1 on failure
 */
int bf_sys_rwlock_trywrlock(bf_sys_rwlock_t *lock);

/**
 * unlock a rw lock
 * @param lock
 *  pointer to lock
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_rwlock_unlock(bf_sys_rwlock_t *lock);

/**
 * initialize a rw_mutex_lock
 * @param lock
 *  pointer to rw_mutex_lock  attribute (OS/platform dependent)
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_rw_mutex_lock_init(bf_sys_rw_mutex_lock_t *lock);

/**
 * destroy a rw_mutex_lock
 * @param lock
 *  pointer to rw_mutex_lock
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_rw_mutex_lock_del(bf_sys_rw_mutex_lock_t *lock);

/**
 * read lock a rw_mutex_lock
 * @param lock
 *  pointer to bf_sys_rw_mutex_lock_t
 * @return Status
 *  0 on Success(may block), implementation specific error on failure
 */
int bf_sys_rw_mutex_lock_rdlock(bf_sys_rw_mutex_lock_t *lock);

/**
 * write lock a rw_mutex_lock
 * @param lock
 *  pointer to bf_sys_rw_mutex_lock_t
 * @return Status
 *  0 on Success(may block), implementation specific error on failure
 */
int bf_sys_rw_mutex_lock_wrlock(bf_sys_rw_mutex_lock_t *lock);

/**
 * unlock a rw_mutex_lock after read
 * @param lock
 *  pointer to lock
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_rw_mutex_lock_rdunlock(bf_sys_rw_mutex_lock_t *lock);

/**
 * unlock a rw_mutex_lock after write
 * @param lock
 *  pointer to lock
 * @return Status
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_rw_mutex_lock_wrunlock(bf_sys_rw_mutex_lock_t *lock);

/**
 * perform an atomic compare and swap. If the current value of var is same
 * as that of the old_val, then write the new_val to var
 * @param var
 *  pointer to value to compare
 * @param old_val
 *  old value to be compared with
 * @param new_val
 *  new value to be written
 * @return int
 *  1 if the comparison was successful and the new_val was written to var,
 *  0 otherwise
 */
int bf_sys_compare_and_swap(bf_sys_cmp_and_swp_t *var,
                            bf_sys_cmp_and_swp_t old_val,
                            bf_sys_cmp_and_swp_t new_val);

/* @} */

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* _BF_SYS_SEM_H_ */
