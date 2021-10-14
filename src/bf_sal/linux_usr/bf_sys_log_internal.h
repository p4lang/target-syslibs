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
int bf_sys_log_array(
    int module, int bf_level, const char *indent, uint8_t *array, size_t len);

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
