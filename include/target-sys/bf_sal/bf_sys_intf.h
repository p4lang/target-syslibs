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
#ifndef BF_SYS_INTF_H_INCLUDED
#define BF_SYS_INTF_H_INCLUDED

#include "bf_sys_mem.h"
#include "bf_sys_str.h"
#include "bf_sys_sem.h"
#include "bf_sys_thread.h"
#include "bf_sys_timer.h"
#include "bf_sys_log.h"
#include "bf_sys_assert.h"
#include "bf_sys_dma.h"

const char *bf_syslib_get_version(void);
const char *bf_syslib_get_internal_version(void);

#endif /* BF_SYS_INTF_H_INCLUDED */
