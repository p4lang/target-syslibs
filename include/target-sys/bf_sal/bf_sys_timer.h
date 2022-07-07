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
 * @file bf_sys_timer.h
 * \brief bf_sal timer management
 * @date
 *
 */

#ifndef _BF_SYS_TIMER_H_
#define _BF_SYS_TIMER_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup bf_sal-timer
 * @{
 */

/*!
 * Enum bf sys timer status
 */
typedef enum bf_sys_timer_status_t {
  BF_SYS_TIMER_OK = 0,
  BF_SYS_TIMER_INVALID_ARG,
  BF_SYS_TIMER_NO_RESOURCES,
  BF_SYS_TIMER_NOT_INITED
} bf_sys_timer_status_t;

struct bf_sys_timer_s;

/**
 * bf_sys timer timeout callback
 */
typedef void (*bf_sys_timeout_cb)(struct bf_sys_timer_s *timer, void *data);

/*!
 * structure bf sys timer
 */
typedef struct bf_sys_timer_s {
  void *timer; /* OS abstracted context pointer */
  bf_sys_timeout_cb cb_fn;
  void *cb_data;
} bf_sys_timer_t;

/**
 * create a timer
 * @param timer
 *  timer to  create
 * @param start_msecs
 *  milliseconds to first start (timer configuration)
 * @param period_msecs
 *  milliseconds to repeat (timer configuration)
 * @param cb_fn
 *  timeout call back function
 * @param cb_data
 *  parameter passed to timeout callback function
 * @return
 *  BF_SYS_TIMER_OK on success, error code on failure
 */
bf_sys_timer_status_t bf_sys_timer_create(bf_sys_timer_t *timer,
                                          uint32_t start_msecs,
                                          uint32_t period_msecs,
                                          bf_sys_timeout_cb cb_fn,
                                          void *cb_data);

/**
 * start a timer
 * @param timer
 *  timer to start
 * @return
 *  BF_SYS_TIMER_OK on success, error code on failure
 */
bf_sys_timer_status_t bf_sys_timer_start(bf_sys_timer_t *timer);

/**
 * stop a timer
 * @param timer
 *  timer to stop
 * @return
 *  BF_SYS_TIMER_OK on success, error code on failure
 */
bf_sys_timer_status_t bf_sys_timer_stop(bf_sys_timer_t *timer);

/**
 * delete a timer
 * @param timer
 *  timer to delete
 * @return
 *  BF_SYS_TIMER_OK on success, error code on failure
 */
bf_sys_timer_status_t bf_sys_timer_del(bf_sys_timer_t *timer);

/**
 * initialize a timer subsystem Never-ending function.
 * @return
 *  BF_SYS_TIMER_OK on success, error code on failure
 */
bf_sys_timer_status_t bf_sys_timer_init(void);

/* sleep and delay functions */

/**
 * sleep in seconds
 * @param seconds
 *  seconds to sleep
 * @return
 *  0 if returns after sleeping, or leftover seconds if it woke up due
 *    to something
 */
unsigned int bf_sys_sleep(int seconds);

/**
 * sleep in microseconds
 * @param micro_seconds
 *  micro seconds to sleep
 * @return
 *  0 if successful, -1 on error
 */
int bf_sys_usleep(int micro_seconds);

/* @} */

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* _BF_SYS_TIMER_H_ */
