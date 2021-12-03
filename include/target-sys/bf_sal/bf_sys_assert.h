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
 * @file bf_sys_assert.h
 * @date
 *
 *
 */
#ifndef _BF_SYS_ASSERT_H_
#define _BF_SYS_ASSERT_H_

#ifndef __KERNEL__

#ifndef bf_sys_assert
#include <assert.h>
#include <execinfo.h>
#include "bf_sys_log.h"

#define bf_sys_assert(x)                                                      \
  do {                                                                        \
    int assert_val = !!(x);                                                   \
    if (!assert_val) {                                                        \
      void *backtrace_buf[128];                                               \
      int backtrace_depth = backtrace(backtrace_buf, 128);                    \
      char **backtrace_strings =                                              \
          backtrace_symbols(backtrace_buf, backtrace_depth);                  \
      if (backtrace_strings) {                                                \
        int backtrace_i;                                                      \
        for (backtrace_i = 0; backtrace_i < backtrace_depth; ++backtrace_i) { \
          bf_sys_log_and_trace(                                               \
              BF_MOD_SYS, BF_LOG_ERR, "%s", backtrace_strings[backtrace_i]);  \
        }                                                                     \
      }                                                                       \
    }                                                                         \
    assert(x);                                                                \
  } while (0)
#endif

#ifndef bf_sys_dbgchk
#include "bf_sys_log.h"
#ifdef __cplusplus
extern "C" {
#endif
extern int bf_sys_dbg_mode();
#ifdef __cplusplus
}
#endif /* C++ */
#define bf_sys_dbgchk_str(x) #x
#define bf_sys_dbgchk(x)                                             \
  {                                                                  \
    if (bf_sys_dbg_mode()) {                                         \
      bf_sys_assert(x);                                              \
    } else if (!(x)) {                                               \
      bf_sys_log_and_trace(BF_MOD_SYS,                               \
                           BF_LOG_ERR,                               \
                           "ASSERTION FAILED: \"" bf_sys_dbgchk_str( \
                               x) "\" (" #x ") from %s:%d",          \
                           __func__,                                 \
                           __LINE__);                                \
    }                                                                \
  }
#endif

#endif /* __KERNEL */

#endif /* _BF_SYS_ASSERT_H_ */
