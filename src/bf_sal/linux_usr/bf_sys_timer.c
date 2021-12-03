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
 * @file bf_sys_timer.c
 * @date
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#include <libev/ev.h>
#include <target-sys/bf_sal/bf_sys_timer.h>
#include <target-sys/bf_sal/bf_sys_mem.h>

typedef struct {
  ev_async async_w;
  pthread_mutex_t lock;
  int inited;
  struct ev_loop *loop;
} userdata;

userdata u;

static void timeout_cb(EV_P_ ev_timer *w, int revents) {
  /* Call the users callback with data */
  bf_sys_timer_t *t = (bf_sys_timer_t *)w->data;
  t->cb_fn(t, t->cb_data);
  (void)EV_A;
  (void)revents;
}

bf_sys_timer_status_t bf_sys_timer_create(bf_sys_timer_t *t,
                                          uint32_t start_msecs,
                                          uint32_t period_msecs,
                                          bf_sys_timeout_cb cb_fn,
                                          void *cb_data) {
  ev_timer *evt;
  double start = (double)start_msecs / 1000;
  double period = (double)period_msecs / 1000;

  if ((t == NULL) || (cb_fn == NULL)) {
    return BF_SYS_TIMER_INVALID_ARG;
  }

  evt = (ev_timer *)bf_sys_malloc(sizeof(ev_timer));
  if (!evt) {
    return BF_SYS_TIMER_NO_RESOURCES;
  }

  evt->data = t;
  t->cb_fn = cb_fn;
  t->cb_data = cb_data;
  t->timer = (void *)evt;
  ev_timer_init(evt, timeout_cb, start, period);
  return BF_SYS_TIMER_OK;
}

bf_sys_timer_status_t bf_sys_timer_start(bf_sys_timer_t *t) {
  ev_timer *evt = (ev_timer *)t->timer;
  if (!u.inited) {
    return BF_SYS_TIMER_NOT_INITED;
  }
  pthread_mutex_lock(&u.lock);
  ev_timer_start(u.loop, evt);
  ev_async_send(u.loop, &u.async_w);
  pthread_mutex_unlock(&u.lock);
  return BF_SYS_TIMER_OK;
}

bf_sys_timer_status_t bf_sys_timer_stop(bf_sys_timer_t *t) {
  ev_timer *evt = (ev_timer *)t->timer;
  if (!u.inited) {
    return BF_SYS_TIMER_NOT_INITED;
  }
  pthread_mutex_lock(&u.lock);
  ev_timer_stop(u.loop, evt);
  ev_async_send(u.loop, &u.async_w);
  pthread_mutex_unlock(&u.lock);
  return BF_SYS_TIMER_OK;
}

bf_sys_timer_status_t bf_sys_timer_del(bf_sys_timer_t *t) {
  if (t == NULL) {
    return BF_SYS_TIMER_INVALID_ARG;
  }

  bf_sys_timer_stop(t);

  bf_sys_free(t->timer);

  t->cb_fn = NULL;
  t->timer = NULL;
  return BF_SYS_TIMER_OK;
}

static void async_cb(EV_P_ ev_async *w, int revents) {
  (void)EV_A;
  (void)revents;
  (void)w;
  /* Do nothing */
}

static void l_release(EV_P) {
  pthread_mutex_unlock(&u.lock);
  (void)EV_A;
}

static void l_acquire(EV_P) {
  pthread_mutex_lock(&u.lock);
  (void)EV_A;
}

/** Never-ending function. */
bf_sys_timer_status_t bf_sys_timer_init(void) {
  u.loop = EV_DEFAULT; /* or ev_default_loop (0); */

  if (!u.loop) return BF_SYS_TIMER_NO_RESOURCES;

/* dereferencing type-punned pointer will break strict-aliasing rules
 * so, apply temporary GCC diagnostics pragma
 * the possibly breakage is in the third party header file, ev.h
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
  ev_async_init(&u.async_w, async_cb);
#pragma GCC diagnostic pop

  ev_async_start(u.loop, &u.async_w);

  u.inited = 1;

  pthread_mutex_init(&u.lock, 0);

  ev_set_loop_release_cb(u.loop, l_release, l_acquire);

  l_acquire(u.loop);
  ev_loop(u.loop, 0);
  l_release(u.loop);
  return BF_SYS_TIMER_OK;
}

unsigned int bf_sys_sleep(int seconds) { return (sleep(seconds)); }

int bf_sys_usleep(int micro_seconds) { return (usleep(micro_seconds)); }

int bf_sys_settimeofday(int sec, int usec) {
  struct timeval tv;

  tv.tv_sec = sec;
  tv.tv_usec = usec;
  return (settimeofday(&tv, NULL));
}

int bf_sys_gettimeofday(int *sec, int *usec) {
  struct timeval tv;

  if (gettimeofday(&tv, NULL) == 0) {
    *sec = tv.tv_sec;
    *usec = tv.tv_usec;
    return 0;
  } else {
    return -1;
  }
}

int bf_sys_setclocktime(int sec, int nsec) {
  struct timespec ts;

  ts.tv_sec = sec;
  ts.tv_nsec = nsec;
  return (clock_settime(CLOCK_MONOTONIC, &ts));
}

int bf_sys_getclocktime(int *sec, int *nsec) {
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
    *sec = ts.tv_sec;
    *nsec = ts.tv_nsec;
    return 0;
  } else {
    return -1;
  }
}
