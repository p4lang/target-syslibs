/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) 2015-2019 Barefoot Networks, Inc.

 * All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains the property of
 * Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
 * technical concepts contained herein are proprietary to Barefoot Networks,
 * Inc.
 * and its suppliers and may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material is
 * strictly forbidden unless prior written permission is obtained from
 * Barefoot Networks, Inc.
 *
 * No warranty, explicit or implicit is provided, unless granted under a
 * written agreement with Barefoot Networks, Inc.
 *
 * $Id: $
 *
 ******************************************************************************/
/*!
 * @file bf_sys_thread.h
 * @date
 *
 *
 */
#ifndef _BF_SYS_THREAD_H_
#define _BF_SYS_THREAD_H_

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup bf_sal-thread
 * @{
 */

/**
 * thread function
 */
typedef void *(*thread_fn)(void *);

/**
 * OS/kernel abstracted thread typedef
 */
typedef void *bf_sys_thread_t;

/**
 * create a thread
 * @param thr
 *  pointer to bf_sys_thread_t
 * @param fn
 *  thread function
 * @param arg
 *  argument to pass to thread function
 * @param flag
 *  flags (OS/kernel specific)
 * @return
 *  0 on Success, error code on failure
 */
int bf_sys_thread_create(bf_sys_thread_t *thr,
                         thread_fn fn,
                         void *arg,
                         int flags);

/**
 * exit  the current thread
 * @param status
 *   exit status (available to whoever joins it)
 * @return
 *  none
 */
void bf_sys_thread_exit(void *status);

/**
 * cancel  a thread
 * @param thr
 *  pointer to bf_sys_thread_t of thread to be cancelled
 * @return
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_thread_cancel(bf_sys_thread_t thr);

/**
 * detach  a thread
 * @param thr
 *  pointer to bf_sys_thread_t of thread to be detached
 * @return
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_thread_detach(bf_sys_thread_t thr);

/**
 * cancel  a thread and join it
 * @param thr
 *  pointer to bf_sys_thread_t of thread to be cancelled
 * @param status
 *   exit status of cancelled thread
 * @return
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_thread_join(bf_sys_thread_t thr, void **status);

/**
 * thread idof itself
 * @param
 *  none
 * @return
 *  bf_sys_thread_t of itself
 */
bf_sys_thread_t bf_sys_thread_self(void);

/**
 * set thread name.
 * @param thr
 *  pointer to bf_sys_thread_t
 * @param name
 *  name of the thread
 * @return
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_thread_set_name(bf_sys_thread_t thr, const char *name);

/**
 * Yield current thread
 * @return
 *  0 on Success, implementation specific error on failure
 */
int bf_sys_thread_yield(void);

/* @} */

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* _BF_SYS_THREAD_H_ */
