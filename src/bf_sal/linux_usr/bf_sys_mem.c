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
