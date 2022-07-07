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
 * @file bf_sys_thread.c
 * @date
 *
 *
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <target-sys/bf_sal/bf_sys_thread.h>
#include <unistd.h>

int bf_sys_thread_create(bf_sys_thread_t *thr, thread_fn fn, void *arg,
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

int bf_sys_thread_yield(void) { return sched_yield(); }
