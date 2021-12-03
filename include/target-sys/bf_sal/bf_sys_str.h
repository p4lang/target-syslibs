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
 * @file bf_sys_str.h
 * @date
 *
 *
 */
#ifndef _BF_SYS_STR_H_
#define _BF_SYS_STR_H_

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Duplicate a string
 * @param c
 *  Source string to copy
 * @return
 *  Pointer to a copy of the string, must be freed by the caller with
 *  bf_sys_free.
 *  Returns NULL if memory cannot be allocated for the copy.
 */
char *bf_sys_strdup(const char *c);

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* _BF_SYS_STR_H_ */
