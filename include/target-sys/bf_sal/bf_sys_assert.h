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
 * @file bf_sys_assert.h
 * @date
 *
 *
 */
#ifndef _BF_SYS_ASSERT_H_
#define _BF_SYS_ASSERT_H_

#ifndef __KERNEL__

#ifndef bf_sys_assert
#include "bf_sys_log.h"
#include <assert.h>
#include <execinfo.h>

#define bf_sys_assert(x)                                                       \
  do {                                                                         \
    int assert_val = !!(x);                                                    \
    if (!assert_val) {                                                         \
      void *backtrace_buf[128];                                                \
      int backtrace_depth = backtrace(backtrace_buf, 128);                     \
      char **backtrace_strings =                                               \
          backtrace_symbols(backtrace_buf, backtrace_depth);                   \
      if (backtrace_strings) {                                                 \
        int backtrace_i;                                                       \
        for (backtrace_i = 0; backtrace_i < backtrace_depth; ++backtrace_i) {  \
          bf_sys_log_and_trace(BF_MOD_SYS, BF_LOG_ERR, "%s",                   \
                               backtrace_strings[backtrace_i]);                \
        }                                                                      \
      }                                                                        \
    }                                                                          \
    assert(x);                                                                 \
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
#define bf_sys_dbgchk(x)                                                       \
  {                                                                            \
    if (bf_sys_dbg_mode()) {                                                   \
      bf_sys_assert(x);                                                        \
    } else if (!(x)) {                                                         \
      bf_sys_log_and_trace(BF_MOD_SYS, BF_LOG_ERR,                             \
                           "ASSERTION FAILED: \"" bf_sys_dbgchk_str(           \
                               x) "\" (" #x ") from %s:%d",                    \
                           __func__, __LINE__);                                \
    }                                                                          \
  }
#endif

#endif /* __KERNEL */

#endif /* _BF_SYS_ASSERT_H_ */
