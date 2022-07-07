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
 * @file bf_sys_log_internal.h
 * @date
 *
 */

#ifndef _BF_SYS_LOG_INTERNAL_H_
#define _BF_SYS_LOG_INTERNAL_H_

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup bf_sal-logging
 * @{
 */

/*!
 * Enum bf log format
 */
typedef enum { bf_log_plain = 0, bf_log_timestamp } bf_log_format;

/**
 *  arary log function
 *
 * @param module
 *  log module (or facility)
 * @param bf_level
 *  log level
 * @param indent
 *  array identifier name
 * @param array
 *  pointer to array
 * @param len
 *  size of array
 * @return
 *  0 on Sucess, -1 on error
 */
int bf_sys_log_array(int module, int bf_level, const char *indent,
                     uint8_t *array, size_t len);

/**
 * set log destination
 *
 * @param module
 *  log module (or facility)
 * @param dest_flag
 *  OR of valid log destinations (BF_LOG_DEST_xxx)
 * @return
 *  0 on Sucess, -1 on error
 */
void bf_sys_set_log_dest(int module, int dest_flag);

/**
 * set log file
 *
 * @param module
 *  log module (or facility)
 * @param file_name
 *  full path of log file name
 * @return
 *  0 on Sucess, -1 on error
 */
void bf_sys_set_log_file(int module, const char *file_name);

/* @} */

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* _BF_SYS_LOG_INTERNAL_H_ */
