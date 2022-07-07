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
