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
 * @file bf_sys_mem.h
 * @date
 *
 *
 */
#ifndef _BF_SYS_MEM_H_
#define _BF_SYS_MEM_H_

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * @addtogroup bf_sal-mem
 * @{
 */

/**
 * allocate memory
 * @param size
 *  num bytes to allocate
 * @return
 *  pointer to allocated memory on success, void on error
 */
void *bf_sys_malloc(size_t size);

/**
 * re allocate memory
 * @param ptr
 *  previously allocated memory
 * @param size
 *  new size of memory
 * @return
 *  pointer to newly allocated memory on success, void on error
 */
void *bf_sys_realloc(void *ptr, size_t size);
/**
 * allocate memory and sets it to zero
 * @param size
 * @param elem
 *  allocate array of elem , size bytes each
 * @return
 *  pointer to allocated memory on success, void on error
 */
void *bf_sys_calloc(size_t elem, size_t size);

/**
 * free memory
 * @param ptr
 *  pointer to allocated memory
 * @return
 *  none
 */
void bf_sys_free(void *ptr);

/* @} */

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* _BF_SYS_MEM_H_ */
