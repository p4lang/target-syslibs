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
#include <stddef.h>

#ifdef BF_SYS_LIBS_USE_TCMALLOC

#include <gperftools/tcmalloc.h>
void *bf_sys_malloc(size_t size) { return tc_malloc(size); }
void *bf_sys_realloc(void *ptr, size_t size) { return tc_realloc(ptr, size); }
void *bf_sys_calloc(size_t elem, size_t size) { return tc_calloc(elem, size); }
void bf_sys_free(void *ptr) { tc_free(ptr); }

#else

#include <stdlib.h>
void *bf_sys_malloc(size_t size) { return malloc(size); }
void *bf_sys_realloc(void *ptr, size_t size) { return realloc(ptr, size); }
void *bf_sys_calloc(size_t elem, size_t size) { return calloc(elem, size); }
void bf_sys_free(void *ptr) { free(ptr); }

#endif
