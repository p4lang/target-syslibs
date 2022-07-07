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

#ifndef BF_SYS_VER_H
#define BF_SYS_VER_H

#include "bf_sys_bld_ver.h"

#define TARGET_SYSLIB_REL_VER "1.0.0"
#define TARGET_SYSLIB_VER TARGET_SYSLIB_REL_VER "-" TARGET_SYSLIB_BLD_VER

#define TARGET_SYSLIB_INTERNAL_VER                                             \
  TARGET_SYSLIB_VER "(" TARGET_SYSLIB_GIT_VER ")"

#endif /* BF_SYS_VER_H */
