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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include <target-sys/bf_sal/bf_sys_dma.h>

#define DMA_POOL_CNT 20
#define DMA_BUF_MAX_CNT 3200
#define DMA_BUF_SIZE 4096

/* buffers in a particular pool, randomly initialized */
static unsigned int buff_cnt[DMA_POOL_CNT] = {
    1, 3200, 512, 511, 63, 7, 7, 96, 0, 15, 15, 3, 15, 127, 7, 5, 5, 63, 63, 1};
/* buffer size in a particular pool, randomly initialized */
static unsigned int buff_size[DMA_POOL_CNT] = {
    1074240, 16384, 8192, 32768, 4096, 4096, 8192, 8192, 64,   16384,
    4096,    32768, 1024, 1024,  1024, 1024, 4096, 8192, 8192, 8};

static void *v_addr[DMA_POOL_CNT][DMA_BUF_MAX_CNT] = {{NULL}};
static bf_dma_addr_t dma_addr[DMA_POOL_CNT][DMA_BUF_MAX_CNT] = {{0}};
static bf_sys_dma_pool_handle_t hndl[DMA_POOL_CNT] = {NULL};
static char dma_pool_name[DMA_POOL_CNT][16];

static int test_dma_mem(void) {
  uint8_t *cptr;
  unsigned int i;

  /* try to write and read to a couple of buffers */
  /* write to buff[2][3] */
  cptr = v_addr[2][3];
  for (i = 0; i < buff_size[2]; i++) {
    *cptr++ = (uint8_t)i;
  }
  /* write to buff[12][0] */
  cptr = v_addr[12][0];
  for (i = 0; i < buff_size[12]; i++, cptr++) {
    *cptr = (uint8_t)(i + 5);
  }
  /* read back and verify */
  /* read buff[2][3] */
  cptr = v_addr[2][3];
  for (i = 0; i < buff_size[2]; i++, cptr++) {
    if (*cptr != (uint8_t)i) {
      printf("mismatch in buff 2/3 read %x expected %x\n", *cptr, (uint8_t)i);
      return -1;
    }
  }
  /* read buff[12][0] */
  cptr = v_addr[12][0];
  for (i = 0; i < buff_size[12]; i++, cptr++) {
    if (*cptr != (uint8_t)(i + 5)) {
      printf("mismatch in buff 12/0 read %x expected %x\n", *cptr,
             (uint8_t)(i + 5));
      return -1;
    }
  }
  printf("buff read/write test OK\n");
  return 0;
}

static int test_dma_2_virt(void) {
  unsigned int i, j;
  void *virtaddr;
  int dma_pool_id;

  /* given the physical IO bus address get back the virtual address for
     for the buffers */
  for (dma_pool_id = 0; dma_pool_id < DMA_POOL_CNT; dma_pool_id++) {
    for (i = 0; i < buff_cnt[dma_pool_id]; i++) {
      for (j = 0; j < buff_size[dma_pool_id]; j++) {
        virtaddr =
            bf_mem_dma2virt(hndl[dma_pool_id], dma_addr[dma_pool_id][i] + j);
        if (((unsigned long)virtaddr - (unsigned long)v_addr[dma_pool_id][i]) !=
            j) {
          printf("Failed to get the virtual address for the dma address: "
                 "%016lx\n",
                 (unsigned long)dma_addr[dma_pool_id][i] + j);
          printf("DMA to virtual address translation test FAIL\n");
          return -1;
        }
      }
    }
  }
  printf("DMA to virtual address translation test OK\n");
  return 0;
}

static int dma_mem_test() {
  int ret, i, j;
  int result = 0;

  ret = bf_sys_dma_lib_init((void *)2, (void *)0x3, NULL);
  if (ret < 0) {
    printf("cannot init EAL\n");
    return (-1);
  }

  for (i = 0; i < DMA_POOL_CNT; i++) {
    sprintf(dma_pool_name[i], "testpool%d", i);
  }

  for (i = 0; i < DMA_POOL_CNT; i++) {
    ret = bf_sys_dma_pool_create(dma_pool_name[i], &hndl[i], 0, 0, buff_size[i],
                                 buff_cnt[i], 256);
    if (ret < 0) {
      printf("cannot alloc pool iteration %d\n", i);
      result = -1;
      goto free_dma_buff;
    }

    for (j = 0; j < (int)buff_cnt[i]; j++) {
      ret = bf_sys_dma_alloc(hndl[i], buff_size[i], &v_addr[i][j],
                             &dma_addr[i][j]);
      if (ret < 0) {
        printf("cannot alloc buff iteration %d %d\n", i, j);
        result = -1;
        goto free_dma_buff;
      } else {
#if 0
                printf("dma buff alloc %p phy_adr %jx for pool %d buff %d\n",
                  v_addr[i][j], p_addr, i, j);
#endif
      }
    }
  }
  printf("DMA pools/buffers allocated OK\n");

  result = test_dma_mem();
  if (result != 0) {
    goto free_dma_buff;
  }

  result = test_dma_2_virt();

free_dma_buff:
  for (i = 0; i < DMA_POOL_CNT; i++) {
    if (hndl[i] == NULL) {
      continue;
    }
    for (j = (int)buff_cnt[i] - 1; j >= 0; j--) {
      if (v_addr[i][j] == NULL) {
        continue;
      }
      bf_sys_dma_free(hndl[i], v_addr[i][j]);
    }
  }
  for (i = 0; i < DMA_POOL_CNT; i++) {
    if (hndl[i] == NULL) {
      continue;
    }
    bf_sys_dma_pool_destroy(hndl[i]);
  }
  return (result);
}

int main() {
  assert(dma_mem_test() == 0);
  return 0;
}
