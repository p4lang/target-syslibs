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
