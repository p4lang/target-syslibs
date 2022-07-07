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
