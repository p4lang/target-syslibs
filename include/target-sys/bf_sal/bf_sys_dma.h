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
 * @file bf_sys_dma.h
 * @date
 *
 *
 */

#ifndef _BF_SYS_DMA_H__
#define _BF_SYS_DMA_H__

/* Allow the use in C++ code.  */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stddef.h>
#include <stdint.h>
#endif /* __KERNEL */

#define BF_HUGE_PAGE_SIZE (2 * 1024 * 1024)

typedef uint64_t bf_phys_addr_t;
typedef bf_phys_addr_t bf_dma_addr_t;

/* external modules register following dma bus map functions */
typedef int (*bf_dma_bus_map)(int dev_id, uint32_t subdev_id, void *phy_addr,
                              size_t size, void **dma_addr);
typedef int (*bf_dma_bus_unmap)(int dev_id, uint32_t subdev_id, void *dma_addr,
                                size_t size);

/**
 * @addtogroup bf_dma-dma
 * @{
 */

#define BF_SYS_DMA_POOL_MAX 21

/**
 * dma pool id type abstraction
 */
typedef void *bf_sys_dma_pool_handle_t;

/**
 * dma data direction
 */
typedef enum {
  BF_DMA_FROM_CPU,
  BF_DMA_TO_CPU,
  BF_DMA_BI_DIRECTIONAL
} bf_sys_dma_dir_t;

/* register the static dma bus map functions
 */
void bf_sys_dma_map_fn_register(bf_dma_bus_map fn1, bf_dma_bus_unmap fn2);

/**
 * Perform platform specific initialization for DMA memory mgmt
 * @param param1 plaform specific parameter
 * @param param2 plaform specific parameter
 * @param param3 plaform specific parameter
 * @return Status 0 on Success, -1 on failure
 */
int bf_sys_dma_lib_init(void *param1, void *param2, void *param3);

/**
 * Create a DMA memory pool
 * @param pool_name name of the pool
 * @param hndl returns pool handle for future pool operations
 * @param dev_id bf device id
 * @param subdev_id bf subdevice id
 * @param cnt number of buffer count in pool
 * @param size size in bytes of each buffer in the pool
 * @param align pool alignment must be power of 2
 * @return Status 0 on Success, -1 on failure
 */
int bf_sys_dma_pool_create(char *pool_name, bf_sys_dma_pool_handle_t *hndl,
                           int dev_id, uint32_t subdv_id, size_t size, int cnt,
                           unsigned align);

/**
 * Destroy a DMA memory pool
 * @param hndl pool handle
 * @return none
 */
void bf_sys_dma_pool_destroy(bf_sys_dma_pool_handle_t hndl);

/**
 * Given the virtual address return the physical address
 * @param virtaddr virtual address of the buffer
 * @return physical address of the buffer
 */
bf_phys_addr_t bf_mem_virt2phy(const void *virtaddr);

/**
 * Given the virtual address return the physical IO bus address
 * by using DMA mapping
 * @param virtaddr virtual address of the buffer
 * @return physical IO bus address of the buffer
 */
bf_dma_addr_t bf_mem_virt2dma(const void *virtaddr);

/**
 * Given the physical IO bus address return the virtual address
 * @param hndl pool handle
 * @param dma_addr physical IO bus address of buffer
 * @return virtual address of the buffer
 */
void *bf_mem_dma2virt(bf_sys_dma_pool_handle_t hndl, bf_dma_addr_t dma_addr);

/**
 * Allocate a buffer from a DMA memory pool
 * @param hndl pool handle
 * @param size size in bytes of the buffer to allocate
 * @param v_addr  pointer to virtual address of buffer
 * @param phys_addr  physical address of buffer
 * @return Status 0 on Success, -1 on failure
 */
int bf_sys_dma_alloc(bf_sys_dma_pool_handle_t hndl, size_t size, void **v_addr,
                     bf_phys_addr_t *phys_addr);

/**
 * get the physical address from the cached values of a DMA memory pool
 * @param hndl pool handle
 * @param v_addr  virtual address of buffer
 * @param phys_addr  physical address of buffer
 * @return Status 0 on Success, -1 on failure
 *
 *  NOTE:  The implementation of this function could generate the physical
 *         address of a vitual address in however way. This implementation
 *         derives it by computing from the cached values for the sake
 *         of performance.
 */
int bf_sys_dma_get_phy_addr_from_pool(bf_sys_dma_pool_handle_t hndl,
                                      void *v_addr, bf_phys_addr_t *phy_addr);

/**
 * Allocate a buffer from a DMA memory pool
 * @param hndl pool handle
 * @param v_addr  pointer to virtual address of buffer
 * @return Status Logical buffer index on success where the
 *                logical buffer index is the index of the buffer in the pool,
 *                for example, if the pool had 128 buffers, logical buffer
 *                index would be in the range of 0 to 127.
 */
int bf_sys_dma_buffer_index(bf_sys_dma_pool_handle_t hndl, void *v_addr);

/**
 * Destroys a buffer in a DMA memory pool
 * @param hndl pool handle
 * @param v_addr virtual address of a buffer in the pool
 * @return none
 */
void bf_sys_dma_free(bf_sys_dma_pool_handle_t hndl, void *v_addr);

/* convenient wrapper API if one needs just one buffer in the pool */
/**
 * Allocate a single buffer DMA memory pool
 * @param pool_name name of the pool
 * @param hndl returns pool handle for future pool operations
 * @param dev_id bf device id
 * @param subdev_id bf subdevice id
 * @param size size in bytes of each buffer in the pool
 * @param v_addr  pointer to virtual address of buffer
 * @param phys_addr  physical address of buffer
 * @return Status 0 on Success, -1 on failure
 */
int bf_sys_dma_buffer_alloc(char *pool_name, bf_sys_dma_pool_handle_t *hndl,
                            int dev_id, uint32_t subdv_id, size_t size,
                            void **v_addr, bf_phys_addr_t *phys_addr);

/**
 * Destroys a single buffer DMA memory pool
 * @param pool_name name of the pool
 * @param v_addr virtual address of a buffer
 * @return none
 */
void bf_sys_dma_buffer_free(bf_sys_dma_pool_handle_t hndl, void *v_addr);

/**
 * bus map a dma buffer
 * @param hndl pool handle to which the buffer belongs
 * @param cpu_vaddr pointer to virtual address of buffer
 * @param phys_addr physical address of the buffer
 * @param size size in bytes of each buffer in the pool to map, must not be
 *   greater than the buffer size
 * @param dma_addr  physical IO bus address of buffer
 * @param direction data DMA direction
 * @return Status 0 on Success, -1 on failure
 *
 * this function is meant to dynamically bus map a buffer. Static mapping
 * is implemented as part of bf_sys_dma_pool_create()
 */
int bf_sys_dma_map(bf_sys_dma_pool_handle_t hndl, const void *cpu_vaddr,
                   const bf_phys_addr_t phys_addr, size_t size,
                   bf_dma_addr_t *dma_addr, bf_sys_dma_dir_t direction);

/**
 * bus unmap a dma buffer
 * @param hndl pool handle to which the buffer belongs
 * @param cpu_vaddr  pointer to virtual address of buffer
 * @param size size in bytes of each buffer in the pool to unmap, must not be
 *   greater than the buffer size
 * @param direction data DMA direction
 * @return Status 0 on Success, -1 on failure
 *
 * this function is meant to dynamically bus unmap a buffer. Static unmapping
 * is implemented as part of bf_sys_dma_pool_destroy()
 */
int bf_sys_dma_unmap(bf_sys_dma_pool_handle_t hndl, const void *cpu_vaddr,
                     size_t size, bf_sys_dma_dir_t direction);

/**
 * cache flush a buffer
 * @param cpu_vaddr  pointer to virtual address of buffer
 * @param size size in bytes to flush
 * @return Status 0 on Success, -1 on failure
 */
int bf_sys_dma_cache_flush(void *cpu_vaddr, size_t size);

/**
 * cache invalidate a buffer
 * @param cpu_vaddr  pointer to virtual address of buffer
 * @param size size in bytes to invalidate
 * @return Status 0 on Success, -1 on failure
 */
int bf_sys_dma_cache_invalidate(void *cpu_vaddr, size_t size);

/* @} */

#ifdef __cplusplus
}
#endif /* C++ */

#endif
