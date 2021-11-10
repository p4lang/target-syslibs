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
#ifndef BF_SYS_VER_H
#define BF_SYS_VER_H

#include "bf_sys_bld_ver.h"

#define TARGET_SYSLIB_REL_VER "1.0.0"
#define TARGET_SYSLIB_VER TARGET_SYSLIB_REL_VER "-" TARGET_SYSLIB_BLD_VER

#define TARGET_SYSLIB_INTERNAL_VER TARGET_SYSLIB_VER "(" TARGET_SYSLIB_GIT_VER ")"

#endif /* BF_SYS_VER_H */
