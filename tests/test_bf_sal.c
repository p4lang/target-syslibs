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

#include <assert.h>
#include <semaphore.h>
#include <stdio.h>
#include <target-sys/bf_sal/bf_sys_intf.h>
#include <unistd.h>

static bf_sys_mutex_t test_mtx;
static bf_sys_timer_t test_timer;
static int err_thr1 = 0, err_thr2 = 0;
static int timer_over = 0;

static int bf_sal_test_mem(void) {
  char *cptr = bf_sys_malloc(10000);
  if (cptr == NULL) {
    printf("malloc error\n");
    return -1;
  }
  bf_sys_free(cptr);
  return 0;
}

void bf_timer_cb(bf_sys_timer_t *timer, void *data) {
  int *iptr = (int *)data;
  int err = -1;

  (void)timer;
  printf("timer val %d\n", (*iptr)++);
  if (*iptr >= 3) {
    timer_over = 1;
    if (bf_sys_timer_stop(&test_timer) != BF_SYS_TIMER_OK) {
      printf("error stopping timer\n");
      bf_sys_thread_exit(&err);
    }
  }
}

#if 0 /* unsed functions */
static int
bf_sal_test_named_mutex (void)
{
    return 0;
}

static int
bf_sal_test_timer (void)
{
    int val = 0;

    if (bf_sys_timer_create(&test_timer, 1000, 500, bf_timer_cb, &val)
                            != BF_SYS_TIMER_OK) {
        printf("error creating timer\n");
        return -1;
    }
#if 1
    if (bf_sys_timer_start(&test_timer) != BF_SYS_TIMER_OK) {
        printf("error starting timer\n");
        return -1;
    }
#endif
    return 0;
}
#endif /* unsed functions */

void *thread_1(void *arg) {
  int keep_running = *(int *)arg;
  int err = -1;

  printf("Starting thread_1\n");
  while (keep_running) {
    if (bf_sys_mutex_lock(&test_mtx)) {
      printf("Error aquiring mutex thread_1\n");
      err_thr1 = 1;
      bf_sys_thread_exit(&err);
    }
    printf("thread1 got lock\n");
    sleep(2);
    if (bf_sys_mutex_unlock(&test_mtx)) {
      printf("Error releasing mutex thread_1\n");
      err_thr1 = 1;
      bf_sys_thread_exit(&err);
    }
    printf("thread1 released lock\n");
    break;
  }

  printf("Stopping thread_1\n");
  err = 0;
  bf_sys_thread_exit(&err);
  return NULL; /* for compiler warnings */
}

void *thread_2(void *arg) {
  int keep_running = *(int *)arg;
  int err = -1;

  printf("Starting thread_2\n");
  sleep(1);
  while (keep_running) {
    if (!bf_sys_mutex_trylock(&test_mtx)) {
      printf("thread_2 should not have acquired mutex\n");
      err_thr2 = 1;
      bf_sys_mutex_unlock(&test_mtx);
      bf_sys_thread_exit(&err);
    }
    sleep(2);
    if (bf_sys_mutex_lock(&test_mtx)) {
      printf("Error aquiring mutex thread_2\n");
      err_thr2 |= 1;
      bf_sys_thread_exit(&err);
    }
    printf("thread2 got lock\n");
    if (bf_sys_mutex_unlock(&test_mtx)) {
      printf("Error releasing mutex thread_2\n");
      err_thr2 |= 1;
      bf_sys_thread_exit(&err);
    }
    printf("thread2 released lock\n");
    break;
  }

  printf("Stopping thread_2\n");
  err = 0;
  bf_sys_thread_exit(&err);
  return NULL; /* for compiler warnings */
}

void *timer_thread(void *arg) {
  (void)arg;
  bf_sys_timer_init();
  return NULL; /* for compiler warnings */
}

/* creat = 1 ; create two thread, 0: delete two threads */
static int bf_sal_test_thread(int create) {
  static bf_sys_thread_t thr1;
  static bf_sys_thread_t thr2;
  int keep_running1 = 1;
  int keep_running2 = 1;
  int res = 0;

  (void)create; /* not used for now */
  res |= bf_sys_thread_create(&thr1, thread_1, &keep_running1, 0);
  bf_sys_thread_set_name(thr1, bf_sal_thr_1");
  res |= bf_sys_thread_create(&thr2, thread_2, &keep_running2, 0);
  bf_sys_thread_set_name(thr1, bf_sal_thr_2");

  if (res) {
    printf("error creating threads\n");
    return -1;
  }
  sleep(5);
  keep_running1 = keep_running2 = 0;
  return 0;
}

static int bf_sal_test_mtx(void) {
  /* intialization */
  bf_sys_mutex_init(&test_mtx);

  /* mutext test using the two threads */
  bf_sal_test_thread(1);

  if (bf_sys_mutex_del(&test_mtx)) {
    printf("error destroying the mutex\n");
    return -1;
  }
  return 0;
}

static int bf_sal_test() {
  bf_sys_thread_t timer_thr;
  int res, val = 0;

  /* create a timer thread */
  res = bf_sys_thread_create(&timer_thr, timer_thread, &val, 0);
  if (res) {
    printf("error creating timer thread\n");
    return -1;
  }
  bf_sys_thread_set_name(timer_thr, "bf_sal_ut_tmr");

  /* mutex test */
  if (bf_sal_test_mtx()) {
    return -1;
  } else {
    printf("mutex test passed\n");
  }

  /* memory test */
  if (bf_sal_test_mem()) {
    printf("error mem test\n");
    return -1;
  } else {
    printf("malloc test passed\n");
  }
#if 0
    /* timer test */

    if (bf_sal_test_timer()) {
        printf("error timer test\n");
        return -1;
    } else {
        printf("timer test passed\n");
    }
    while (timer_over == 0) {
        sleep(1);
    }
#endif
  return 0;
}

int main() {
  assert(bf_sal_test() == 0);
  return 0;
}
