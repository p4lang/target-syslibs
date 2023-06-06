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
 * @file bf_mmap.c
 * @date
 *
 *
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <target-sys/bf_sal/bf_sys_dma.h>
#include <target-sys/bf_sal/bf_sys_mem.h>
#include <unistd.h>

#define BF_INVALID_PHY_ADDR ((bf_phys_addr_t)(0xFFFFFFFFFFFFFFFFULL))
#define BF_INVALID_DMA_ADDR ((bf_dma_addr_t)(0xFFFFFFFFFFFFFFFFULL))

#define POOL_HDR_SIZE (4 * 1024)
#define ALIGN_TO_BF_PAGE_SIZE(x)                                               \
  (((x) + BF_HUGE_PAGE_SIZE - 1) / BF_HUGE_PAGE_SIZE * BF_HUGE_PAGE_SIZE)

/**
 * huge_page info including the virtual and physical IO bus addresses
 */
typedef struct {
  bf_dma_addr_t base_dma_addr; /* physical IO bus address of the huge page */
  void *base_virt_addr;        /* virtual address of the huge page */
} bf_huge_page_info_t;

/* data structures */
typedef struct {
  int pool_inited;     /* 0 if pool is not initialized */
  int pool_id;         /* pool id */
  size_t hdr_size;     /* reserved header size for pool allocation */
  void *pool_ptr;      /* pool's starting address */
  uint8_t *buf_start;  /* starting address of buffers within the pool */
  int buf_cnt;         /* number of buffers in pool */
  size_t buf_size;     /* size of eachbuffer in pool */
  int alignment;       /* application's alignment requirement */
  void **pool_buf_ptr; /* pointer to queue of buf pointers */
  volatile uint32_t
      pool_buf_offset; /* position of current head into the above queue */
  bf_phys_addr_t base_phy_addr; /* physical address of first buffer of pool */
  volatile int pool_gate;       /* assist for atomic operation */
  char name[64];                /* pool name */
  bf_huge_page_info_t *huge_page_info_ptr; /* an array of huge page pointers */
  int num_huge_pages; /* number of huge pages allocated for the DMA memory pool
                       */
  unsigned int pool_hdr_offset; /* offset to be added so that no buffer spills
                                   over from the physical page */
  int fd;     /* file handle of bf device for dma bus mapping */
  int dev_id; /* device id that the pool belongs to */
  uint32_t
      subdev_id; /* subdev_id (within the device id) that the pool belongs to */
} bf_huge_pool_t;

static bf_dma_bus_map bf_sys_dma_map_fn = NULL;
static bf_dma_bus_unmap bf_sys_dma_unmap_fn = NULL;

void bf_sys_dma_map_fn_register(bf_dma_bus_map fn1, bf_dma_bus_unmap fn2) {
  bf_sys_dma_map_fn = fn1;
  bf_sys_dma_unmap_fn = fn2;
}

/**
 * Platform specific init for dma memory mgmt
 * param1 : register function pointer that provides bus mapping services
 */
int bf_sys_dma_lib_init(void *param1, void *param2, void *param3) {
  (void)param1;
  (void)param2;
  (void)param3;
  return 0;
}

/*
 * Get physical address of any mapped virtual address in the
 * current process.
 *
 */
bf_phys_addr_t bf_mem_virt2phy(const void *virtaddr) {
  int fd;
  uint64_t page, phys_addr;
  unsigned long virt_pfn;
  int page_size;
  off_t offset;

  /* standard page size */
  page_size = getpagesize();

  fd = open("/proc/self/pagemap", O_RDONLY);
  if (fd < 0) {
    printf("%s(): cannot open /proc/self/pagemap: %s\n", __func__,
           strerror(errno));
    return BF_INVALID_PHY_ADDR;
  }

  virt_pfn = (unsigned long)virtaddr / page_size;
  offset = sizeof(uint64_t) * virt_pfn;
  if (lseek(fd, offset, SEEK_SET) == (off_t)-1) {
    printf("%s(): seek error in /proc/self/pagemap: %s\n", __func__,
           strerror(errno));
    close(fd);
    return BF_INVALID_PHY_ADDR;
  }

  if (read(fd, &page, sizeof(uint64_t)) < 0) {
    printf("%s(): cannot read /proc/self/pagemap: %s\n", __func__,
           strerror(errno));
    close(fd);
    return BF_INVALID_PHY_ADDR;
  }

  /*
   * the pfn (page frame number) are bits 0-54 (see
   * pagemap.txt in linux Documentation)
   */
  phys_addr = ((page & 0x7fffffffffffffULL) * page_size) +
              ((unsigned long)virtaddr % page_size);
  close(fd);
  return (bf_phys_addr_t)phys_addr;
}

/*
 * Get the physical IO bus address of any mapped virtual address
 * in the current process
 *
 */
bf_dma_addr_t bf_mem_virt2dma(const void *virtaddr) {
  bf_dma_addr_t dma_addr;

  /* Since for the current implementation the physical IO bus address
     and the physical addresses are the same, call the bf_mem_virt2phy()
     function and return the address */

  dma_addr = (bf_dma_addr_t)bf_mem_virt2phy(virtaddr);
  if (dma_addr == BF_INVALID_PHY_ADDR) {
    return BF_INVALID_DMA_ADDR;
  }

  return dma_addr;
}

/* change the value to current or next aligned value.
 *
 * alignment must be powr of 2
 *
 */
static uint32_t conv_to_next_aligned(uint32_t val, uint32_t alignment) {
  if (val & (alignment - 1)) {
    return ((val + alignment) & ~(alignment - 1));
  } else {
    return val;
  }
}

static void *alloc_huge_pages(size_t size, unsigned int header_offset) {
  size_t actual_size;
  char *ptr;
  actual_size = ALIGN_TO_BF_PAGE_SIZE(size + header_offset);
  ptr = (char *)mmap(NULL, actual_size, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE | MAP_HUGETLB |
                         (21 << MAP_HUGE_SHIFT),
                     -1, 0);
  if (ptr == MAP_FAILED) {
    // If 2MB failed, retry with default size
    ptr = (char *)mmap(NULL, actual_size, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS | MAP_POPULATE | MAP_HUGETLB,
                       -1, 0);
  }
  if (ptr == MAP_FAILED) {
    return NULL;
  }
  /* Save actual_size since mmunmap() requires a size parameter */
  *((size_t *)ptr) = actual_size;
  /* Skip the page with metadata and offset*/
  return (ptr + header_offset);
}

/* free "ALL" huge pages belonging to a dma pool */
static void free_huge_pages(int dev_id, uint32_t subdev_id,
                            bf_huge_page_info_t *base_huge_page, void *ptr,
                            unsigned int header_offset) {
  void *huge_ptr;
  size_t actual_size;
  int i;

  if (ptr == NULL) {
    return;
  }
  /* Jump back to the page with metadata */
  huge_ptr = (char *)ptr - header_offset;

  /* Get the original allocation size */
  actual_size = *((size_t *)huge_ptr);
  assert(actual_size != 0);
  assert(actual_size % BF_HUGE_PAGE_SIZE == 0);
  /* call static bus map services thru registered function to unmap bus address
   * for iommu-enabled platforms
   */
  if (bf_sys_dma_unmap_fn) {
    bf_huge_page_info_t *temp_ptr = base_huge_page;
    for (i = 0; i < (int)(actual_size / BF_HUGE_PAGE_SIZE); i++) {
      if (bf_sys_dma_unmap_fn(dev_id, subdev_id,
                              (void *)(uintptr_t)(temp_ptr->base_dma_addr),
                              BF_HUGE_PAGE_SIZE)) {
        printf("error in dma unmap ioctl for dma addr %p\n",
               (void *)(uintptr_t)temp_ptr->base_dma_addr);
        assert(0);
      }
      temp_ptr++;
    }
  }
  munmap(huge_ptr, actual_size);
}

#if 0  /* Uncomment if needed */
/* change the value to current or previous aligned value.
 *
 * alignment must be powr of 2
 *
 */
static uint64_t
conv_to_prev_aligned (uint64_t val, uint64_t alignment)
{
    return (val & ~(alignment-1));
}
#endif /* Uncomment if needed */

/**
 * Log the virtual and physical IO bus addresses of all the huge pages in the
 * DMA memory pool
 * @param dev_id bf device id
 * @param subdev_id bf subdevice id
 * @param fd file handle of bf device that provides dma bus mapping services
 * @param ptr virtual address of the first huge page
 * @param cnt number of hugepages in the DMA memory pool
 */
static bf_huge_page_info_t *log_virt_dma_addr(int dev_id, uint32_t subdev_id,
                                              void *ptr, int cnt) {
  int i;
  char *huge_ptr = (char *)ptr;
  bf_huge_page_info_t *huge_page;

  /* Create the array of huge page info structures */
  huge_page =
      (bf_huge_page_info_t *)bf_sys_calloc(cnt, sizeof(bf_huge_page_info_t));
  if (NULL == huge_page) {
    return NULL;
  }
  /* Initialize the array of pointers */
  for (i = 0; i < cnt; i++) {
    huge_page[i].base_dma_addr = bf_mem_virt2dma(huge_ptr);
    huge_page[i].base_virt_addr = (void *)huge_ptr;
    /* call registered function to map physical to bus address if iommu
     * is enabled, and, additionally, replace physical address with  bus
     * address
     */
    if (bf_sys_dma_map_fn) {
      /* replace the phy address with bus address returned by the kernel */
      if (bf_sys_dma_map_fn(
              dev_id, subdev_id, (void *)(uintptr_t)huge_page[i].base_dma_addr,
              BF_HUGE_PAGE_SIZE, (void **)&(huge_page[i].base_dma_addr))) {
        printf("error in dma map ioctl for phy addr %p\n",
               (void *)(uintptr_t)huge_page[i].base_dma_addr);
        assert(0);
      }
    }
    huge_ptr += BF_HUGE_PAGE_SIZE;
  }

  return huge_page;
}

/**
 * Given the physical IO bus address return the virtual address and
 * @param hndl DMA memory pool handle
 * @param dma_addr physical IO bus address
 */
void *bf_mem_dma2virt(bf_sys_dma_pool_handle_t hndl, bf_dma_addr_t dma_addr) {
  bf_huge_page_info_t *huge_page_info;
  int num_huge_pages, i;
  unsigned long virtaddr = 0;
  bf_huge_pool_t *dma_pool = (bf_huge_pool_t *)hndl;

  assert(dma_pool);

  huge_page_info = dma_pool->huge_page_info_ptr;
  num_huge_pages = dma_pool->num_huge_pages;

  for (i = 0; i < num_huge_pages; i++) {
    if ((dma_addr >= huge_page_info[i].base_dma_addr) &&
        (dma_addr < (huge_page_info[i].base_dma_addr + BF_HUGE_PAGE_SIZE))) {
      /* Found the huge page to which the given address belongs to */
      virtaddr = (unsigned long)(huge_page_info[i].base_virt_addr) +
                 (dma_addr - huge_page_info[i].base_dma_addr);
      break;
    }
  }

  if (i == num_huge_pages) {
    /* Indicates that the given physical address doesn't belong
       to any of the huge pages in the given DMA memory pool */
    return NULL;
  }

  return (void *)virtaddr;
}

/**
 *  Create an aligned DMA memory pool
 *
 *  @param fd file handle of bf device that provides dma bus mapping services
 *  align must be power of 2, caller to ensure.
 */
int bf_sys_dma_pool_create(char *pool_name, bf_sys_dma_pool_handle_t *hndl,
                           int dev_id, uint32_t subdev_id, size_t size, int cnt,
                           unsigned align) {
  bf_huge_pool_t *dma_pool;
  char *buf_ptr;
  void *vhuge, **vbuf_q;
  int i;
  size_t alloced_size;
  void *huge_ptr;
  int num_huge_pages;
  unsigned int header_offset = 0;
  bf_huge_page_info_t *huge_page_info;

  assert(hndl);
  /* check if align is power of 2 */
  if (align & (align - 1)) {
    return -1;
  }

  /* current implementation guarantees correct creation of the pool
   * only if the size of a single buffer is less than a single
   * huge page */
  if (size > BF_HUGE_PAGE_SIZE) {
    return -1;
  }

  /* current implementation of hugepage guarantees POOL_HDR_SIZE (4K)
   * alignment, so, enough to ensure that user requested alignment
   * requirement is met by POOL_HDR_SIZE alignment
   */
  assert(align <= POOL_HDR_SIZE);

  /* modify size to the next alignment boundary */
  size = conv_to_next_aligned(size, align);

  dma_pool = (bf_huge_pool_t *)bf_sys_calloc(1, sizeof(bf_huge_pool_t));
  if (dma_pool == NULL) {
    return -1;
  }

  vbuf_q = bf_sys_calloc(cnt, sizeof(void *));
  if (vbuf_q == NULL) {
    bf_sys_free(dma_pool);
    return -1;
  }

  /* Use extra POOL_HDR_SIZE bytes for allocation metadata
   * ensure that POOL_HDR_SIZE is big enough to hold meta data
   */
  assert(POOL_HDR_SIZE >= sizeof(size_t));
  header_offset = POOL_HDR_SIZE;
  /* offset the allocation of the buffers so that none of the
   * buffers in the memory pool spill over from a physical
   * page
   */
  if (((size * cnt) + header_offset) > BF_HUGE_PAGE_SIZE) {
    /* Change the offset only if the total required size is more than
       the size of a huge page */
    header_offset = size;
    /* modify offset to the next alignment boundary */
    header_offset = conv_to_next_aligned(header_offset, POOL_HDR_SIZE);
  }

  vhuge = alloc_huge_pages(size * cnt, header_offset);
  if (vhuge == NULL) {
    bf_sys_free(dma_pool);
    bf_sys_free(vbuf_q);
    return -1;
  }

  /* Jump back to the page with metadata */
  huge_ptr = (char *)vhuge - header_offset;

  /* Get the original allocation size */
  alloced_size = *((size_t *)huge_ptr);

  /* Get the number of pages that have been allocated */
  num_huge_pages = alloced_size / BF_HUGE_PAGE_SIZE;

  /* maintain the base virtual and physical IO bus addresses of all the
     huge pages in the dma pool */
  huge_page_info =
      log_virt_dma_addr(dev_id, subdev_id, huge_ptr, num_huge_pages);
  if (NULL == huge_page_info) {
    bf_sys_free(dma_pool);
    bf_sys_free(vbuf_q);
    bf_sys_free(vhuge);
    return -1;
  }
  /* init bf_dma_pool struct  and ensure that there are no reasons to
   * fail any more in the rest of the function */
  dma_pool->hdr_size = POOL_HDR_SIZE;
  dma_pool->pool_ptr = vhuge;
  /* with current scheme vhuge would be POOL_HDR_SIZE (4K) aligned
   * so, no further alignment necessary
   */
  dma_pool->dev_id = dev_id;
  dma_pool->subdev_id = subdev_id;
  dma_pool->buf_start = vhuge;
  dma_pool->buf_cnt = cnt;
  dma_pool->buf_size = size;
  dma_pool->alignment = align;
  dma_pool->pool_buf_ptr = vbuf_q;
  dma_pool->pool_buf_offset = 0;
  dma_pool->pool_gate = 0;
  dma_pool->huge_page_info_ptr = huge_page_info;
  dma_pool->num_huge_pages = num_huge_pages;
  dma_pool->pool_hdr_offset = header_offset;

  strncpy(dma_pool->name, pool_name, sizeof(dma_pool->name) - 1);
  /* null terminate the name, just in case */
  dma_pool->name[sizeof(dma_pool->name) - 1] = 0;
  /* get the base physical address of base buffer */
  dma_pool->base_phy_addr = bf_mem_virt2phy(dma_pool->buf_start);

  if (dma_pool->base_phy_addr == BF_INVALID_PHY_ADDR) {
    printf("Error getting DMA buf base physical address\n");
    bf_sys_free(dma_pool);
    bf_sys_free(vbuf_q);
    bf_sys_free(vhuge);
    bf_sys_free(huge_page_info);
    return -1;
  }
  /* intialize the LIFO queue with free buffer pointers */
  buf_ptr = (char *)(dma_pool->buf_start);
  for (i = 0; i < cnt; i++) {
    dma_pool->pool_buf_ptr[i] = (void *)buf_ptr;
    buf_ptr += size;
  }
  dma_pool->pool_inited = 1;
  *hndl = (bf_sys_dma_pool_handle_t)dma_pool;
  return 0;
}

/**
 *  Free a DMA memory pool
 */
void bf_sys_dma_pool_destroy(bf_sys_dma_pool_handle_t hndl) {
  bf_huge_pool_t *dma_pool = (bf_huge_pool_t *)hndl;
  assert(dma_pool);

  /* free the hugepages pool containing the buffers */
  free_huge_pages(dma_pool->dev_id, dma_pool->subdev_id,
                  dma_pool->huge_page_info_ptr, dma_pool->pool_ptr,
                  dma_pool->pool_hdr_offset);
  /* free the queue containing the pointers to buffers */
  bf_sys_free(dma_pool->pool_buf_ptr);
  /* free the array of structures containing the base physical
     and virtual addresses of the huge pages in the memory pool */
  bf_sys_free(dma_pool->huge_page_info_ptr);
  /* finally, free the bf_huge_pool_t struct */
  bf_sys_free(dma_pool);
}

static int bf_pop_free_buf(bf_huge_pool_t *pool, void **buf_ptr) {
  int err = 0;

  /* close the gate to assist atomic operation */
  do {
  } while (__sync_val_compare_and_swap(&pool->pool_gate, 0, 1) == 1);

  if (pool->pool_buf_offset >= (uint32_t)(pool->buf_cnt)) {
    err = -1;
  } else {
    *buf_ptr = (pool->pool_buf_ptr)[pool->pool_buf_offset++];
  }
  /* open the gate */
  __sync_val_compare_and_swap(&pool->pool_gate, 1, 0);
  return err;
}

static int bf_push_free_buf(bf_huge_pool_t *pool, void *buf_ptr) {
  int err = 0;

  /* close the gate to assist atomic operation */
  do {
  } while (__sync_val_compare_and_swap(&pool->pool_gate, 0, 1) == 1);

  if (pool->pool_buf_offset == 0 ||
      pool->pool_buf_offset > (uint32_t)(pool->buf_cnt)) {
    err = -1;
  } else {
    pool->pool_buf_offset--;
    (pool->pool_buf_ptr)[pool->pool_buf_offset] = buf_ptr;
  }

  /* open the gate */
  __sync_val_compare_and_swap(&pool->pool_gate, 1, 0);
  return err;
}

/**
 *  Allocate a buffer from a DMA memory pool
 */
int bf_sys_dma_alloc(bf_sys_dma_pool_handle_t hndl, size_t size, void **v_addr,
                     bf_phys_addr_t *phys_addr) {
  (void)size;
  bf_huge_pool_t *dma_pool = (bf_huge_pool_t *)hndl;

  assert(dma_pool);

  assert(size <= dma_pool->buf_size);

  if (bf_pop_free_buf(dma_pool, v_addr) < 0) {
    *v_addr = NULL;
    *phys_addr = 0;
    return -1;
  }
  if (bf_sys_dma_get_phy_addr_from_pool(dma_pool, *v_addr,
                                        (bf_dma_addr_t *)phys_addr)) {
    printf("Error getting buf physical address\n");
    return -1;
  }
  if (*phys_addr == BF_INVALID_PHY_ADDR) {
    printf("Error bad physical address\n");
    return -1;
  }
  return 0;
}

/**
 *  Map a buffer to an index within the buffer pool.
 */
int bf_sys_dma_buffer_index(bf_sys_dma_pool_handle_t hndl, void *v_addr) {
  char *buff_v_addr;
  char *pool_v_addr;
  ptrdiff_t idx;

  bf_huge_pool_t *dma_pool = (bf_huge_pool_t *)hndl;
  assert(dma_pool);
  buff_v_addr = v_addr;
  pool_v_addr = dma_pool->pool_ptr;
  idx = (buff_v_addr - pool_v_addr) / dma_pool->buf_size;
  assert(idx >= 0 && idx < dma_pool->buf_cnt);
  return idx;
}

/**
 * get the physical address derived from the cached base physical address
 */
int bf_sys_dma_get_phy_addr_from_pool(bf_sys_dma_pool_handle_t hndl,
                                      void *v_addr, bf_phys_addr_t *phy_addr) {
  bf_huge_pool_t *dma_pool = (bf_huge_pool_t *)hndl;
  bf_phys_addr_t base_phy_addr;
  uint8_t *pool_base_vaddr, *base_vaddr;
  size_t delta;
  int page_index;

  assert(dma_pool);
  assert(dma_pool->pool_inited);
  assert(v_addr);
  pool_base_vaddr = dma_pool->buf_start - dma_pool->pool_hdr_offset;
  delta = (size_t)((uint8_t *)v_addr - pool_base_vaddr);
  page_index = delta / BF_HUGE_PAGE_SIZE;
  /* v_addr must belong to this pool */
  assert(page_index < dma_pool->num_huge_pages);
  base_phy_addr = dma_pool->huge_page_info_ptr[page_index].base_dma_addr;
  base_vaddr =
      (uint8_t *)dma_pool->huge_page_info_ptr[page_index].base_virt_addr;
  delta = (size_t)((uint8_t *)v_addr - base_vaddr);
  *phy_addr = base_phy_addr + delta;
  return 0;
}

/**
 *  Free a buffer into a DMA memory pool
 */
void bf_sys_dma_free(bf_sys_dma_pool_handle_t hndl, void *v_addr) {
  bf_huge_pool_t *dma_pool = (bf_huge_pool_t *)hndl;
  assert(dma_pool);
  assert(v_addr);

  bf_push_free_buf(dma_pool, v_addr);
}

/* convenient wrapper APIs if the pool needs just one buffer */

/**
 *  Allocate a single buffer DMA memory pool
 */
int bf_sys_dma_buffer_alloc(char *pool_name, bf_sys_dma_pool_handle_t *hndl,
                            int dev_id, uint32_t subdev_id, size_t size,
                            void **v_addr, bf_phys_addr_t *phys_addr) {
  if (bf_sys_dma_pool_create(pool_name, hndl, dev_id, subdev_id, size, 1, 64) !=
      0) {
    *v_addr = NULL;
    *phys_addr = 0;
    return -1;
  }
  if (bf_sys_dma_alloc(*hndl, size, v_addr, phys_addr) != 0) {
    bf_sys_dma_pool_destroy(*hndl);
    return -1;
  }
  return 0;
}

/**
 *  Free a single buffer DMA memory pool
 */
void bf_sys_dma_buffer_free(bf_sys_dma_pool_handle_t hndl, void *v_addr) {
  bf_sys_dma_free(hndl, v_addr);
  bf_sys_dma_pool_destroy(hndl);
}

/**
 * bus map a buffer
 */
int bf_sys_dma_map(bf_sys_dma_pool_handle_t hndl, const void *cpu_vaddr,
                   const bf_phys_addr_t phys_addr, size_t size,
                   bf_dma_addr_t *dma_addr, bf_sys_dma_dir_t direction) {
  /* Since for our implementation, the physical and the DMA addresses are the
     same,
     just return the physical address as the DMA address */
  *dma_addr = (bf_dma_addr_t)phys_addr;
  (void)cpu_vaddr;
  (void)hndl;
  (void)size;
  (void)direction;
  return 0;
}

/**
 * bus unmap a buffer
 */
int bf_sys_dma_unmap(bf_sys_dma_pool_handle_t hndl, const void *cpu_vaddr,
                     size_t size, bf_sys_dma_dir_t direction) {
  /* nothing to do for this platform */
  (void)hndl;
  (void)cpu_vaddr;
  (void)size;
  (void)direction;
  return 0;
}

int bf_sys_dma_cache_flush(void *cpu_vaddr, size_t size) {
  /* nothing to do for this platform */
  (void)cpu_vaddr;
  (void)size;
  return 0;
}

int bf_sys_dma_cache_invalidate(void *cpu_vaddr, size_t size) {
  /* nothing to do for this platform */
  (void)cpu_vaddr;
  (void)size;
  return 0;
}
