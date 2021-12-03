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
 * @file bf_sys_thread.c
 * @date
 *
 *
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <pthread.h>
#include <target-sys/bf_sal/bf_sys_thread.h>

int bf_sys_thread_create(bf_sys_thread_t *thr,
                         thread_fn fn,
                         void *arg,
                         int flags) {
  pthread_t *th = (pthread_t *)thr;

  (void)flags;
  return (pthread_create(th, NULL, fn, arg));
}

void bf_sys_thread_exit(void *status) { pthread_exit(status); }

int bf_sys_thread_cancel(bf_sys_thread_t thr) {
  pthread_t th = (pthread_t)thr;

  return (pthread_cancel(th));
}

int bf_sys_thread_detach(bf_sys_thread_t thr) {
  pthread_t th = (pthread_t)thr;

  return (pthread_detach(th));
}

int bf_sys_thread_join(bf_sys_thread_t thr, void **status) {
  pthread_t th = (pthread_t)thr;

  return (pthread_join(th, status));
}

bf_sys_thread_t bf_sys_thread_self(void) {
  return ((bf_sys_thread_t)pthread_self());
}

int bf_sys_thread_set_name(bf_sys_thread_t thr, const char *name) {
  pthread_t th = (pthread_t)thr;

  return (pthread_setname_np(th, name));
}

int bf_sys_thread_yield(void) { return pthread_yield(); }
