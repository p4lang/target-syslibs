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
 * @file bf_sys_log.c
 * @date
 *
 *
 */

#include <assert.h>
#include <config.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <target-sys/bf_sal/bf_sys_mem.h>
#include <target-sys/bf_sal/bf_sys_str.h>
#include <unistd.h>

/* use zlog for logging only */
#include <zlog/src/category.h>
#include <zlog/src/zlog.h>

#include "bf_sys_log_internal.h"
#include <target-sys/bf_sal/bf_sys_log.h>

/**
 * bf_sys_log implementation for linux userspace
 * using "TBD" library for tracing and zlog for logging
 */

/**
 * if log_level >= configured trace_level, then, logs would go to trace buffer
 * as well.
 */

#define BF_LOG_MAX_SIZE 1024

/* cached configuration file for zlog library */
#define ZLOG_CFG_FILE "zlog-cfg-cur"
static char *zlog_cur_log_file = NULL;

/* trace level of an individual module */
static int bf_sys_trace_level[BF_MOD_MAX];

/* zlog category for an individual module */
static zlog_category_t *zlog_cat[BF_MOD_MAX];

/* zlog category name  for an individual module */
/* pre defined module names in zlog config file
 * *** IMPORTANT *** maintain the same index as BF_MOD_xxx
 * in bf_sys_log.h
 */
static const char zlog_cat_name[BF_MOD_MAX][32] = {
    "BF_SYS",  "BF_UTIL", "BF_LLD",  "BF_PIPE",   "BF_TM",  "BF_MC",
    "BF_PKT",  "BF_DVM",  "BF_PORT", "BF_AVAGO",  "BF_DRU", "BF_MAP",
    "BF_API",  "BF_SAI",  "BF_PI",   "BF_PLTFM",  "BF_PAL", "BF_PM",
    "BF_KNET", "BF_BFRT", "BF_P4RT", "BF_SWITCHD", "KRNLMON", "OVSP4RT",
    "INFRAP4D"};

int zlog_would_log_at_level(zlog_category_t *category, int level) {
  if (!category)
    return 0;
  if (zlog_category_needless_level(category, level))
    return 0;
  return 1;
}

/**
 * converts bf log level to zlog log level
 */
static int bf_get_zlog_level(int bf_level) {
  switch (bf_level) {
  case BF_LOG_CRIT:
    return ZLOG_LEVEL_FATAL;
  case BF_LOG_ERR:
    return ZLOG_LEVEL_ERROR;
  case BF_LOG_WARN:
    return ZLOG_LEVEL_WARN;
  case BF_LOG_INFO:
    return ZLOG_LEVEL_INFO;
  case BF_LOG_DBG:
    return ZLOG_LEVEL_DEBUG;
  }

  /* there is no "no-log" level in zlog, so, setting to the least important
   * log level
   */
  return ZLOG_LEVEL_DEBUG;
}

/* zlog lib initialization */
static int bf_sys_zlog_init(const char *arg1) {
  const char *cfg_file_name = arg1;
  char *temp_buff;
  int i, err;
  struct stat sb;

  if (NULL == cfg_file_name) {
    cfg_file_name = DEFAULT_ZLOG_CFG_FILE;
  }
  if (zlog_cur_log_file)
    bf_sys_free(zlog_cur_log_file);
  zlog_cur_log_file = bf_sys_strdup(ZLOG_CFG_FILE);

  if (stat(ZLOG_CFG_FILE, &sb) != 0) {
    /* copy the file to a current config file where changes can be made in
     * runtime
     */
    /* Set the length of the command string based on the size of the two file
     * names plus a few more bytes for the fixed part of the command. */
    int len = 10 + strlen(cfg_file_name) + strlen(ZLOG_CFG_FILE);
    temp_buff = bf_sys_malloc(len);
    if (!temp_buff)
      return -1;
    err = snprintf(temp_buff, len, "cp %s %s", cfg_file_name, ZLOG_CFG_FILE);
    if (!(err < 0 || err >= len))
      err = system(temp_buff);
    if (err < 0 || err >= len) {
      printf("error copying zlog config file \"%s\"\n", temp_buff);
      /* fall back to user supplied file name */
      if (zlog_cur_log_file)
        bf_sys_free(zlog_cur_log_file);
      zlog_cur_log_file = bf_sys_strdup(cfg_file_name);
    }
    bf_sys_free(temp_buff);
  }

  if (!zlog_cur_log_file)
    return -1;

  if (zlog_init(zlog_cur_log_file) != 0) {
    printf("error initializing with bf_sys_log file %s\n", zlog_cur_log_file);
    return -1;
  }
  /* create the categories */
  for (i = 0; i < BF_MOD_MAX; i++) {
    zlog_cat[i] = zlog_get_category(zlog_cat_name[i]);
  }
  return 0;
}

int bf_sys_log_zlog_reconfig(const char *cfg_file_name) {
  if (zlog_reload(cfg_file_name) != 0) {
    printf("error initializing bf_sys_log\n");
    return -1;
  }
  return 0;
}

zlog_category_t *bf_sys_log_get_cat(const char *category_name) {
  return (zlog_get_category(category_name));
}

static void bf_sys_zlog_close(void) {
  zlog_fini();
  if (zlog_cur_log_file) {
    bf_sys_free(zlog_cur_log_file);
    zlog_cur_log_file = NULL;
  }
}

static int bf_sys_trace_init(int default_level) {
  int i;

  for (i = 0; i < BF_MOD_MAX; i++) {
    bf_sys_trace_level[i] = default_level;
  }
  return 0;
}

/* arg1 - Path to logging config file.
 * arg2 - Trace level.
 * arg3 - Trace size. */
int bf_sys_log_init(const void *arg1, const void *arg2, const void *arg3) {
  int err;
  (void)arg3;

  err = bf_sys_zlog_init(arg1);
  err |= bf_sys_trace_init((int)(uintptr_t)arg2);
  return err;
}

int bf_sys_log_close(void) {
  bf_sys_zlog_close();
  return 0;
}

int bf_sys_log(int module, int level, const char *format, ...) {
  if (module >= BF_MOD_MAX)
    return -1;
  bool needs_log =
      zlog_would_log_at_level(zlog_cat[module], bf_get_zlog_level(level));

  if (needs_log) {
    va_list v;
    va_start(v, format);
    vzlog(zlog_cat[module], NULL, 0, NULL, 0, 0, bf_get_zlog_level(level),
          format, v);
    va_end(v);
  }
  return 0;
}
int bf_sys_trace(int module, int level, const char *format, ...) {
  if (module >= BF_MOD_MAX)
    return -1;
  int err = 0;
  (void)format;
  bool needs_trace = level <= bf_sys_trace_level[module];

  if (needs_trace) {
    /* TBD add call to tracing functions */
  }
  return err;
}

int bf_sys_log_and_trace(int module, int level, const char *format, ...) {
  if (module >= BF_MOD_MAX)
    return -1;
  int err = 0;
  bool needs_trace = level <= bf_sys_trace_level[module];
  bool needs_log =
      zlog_would_log_at_level(zlog_cat[module], bf_get_zlog_level(level));

  if (needs_trace) {
    /* TBD add call to tracing functions */
  }
  if (needs_log) {
    va_list v;
    va_start(v, format);
    vzlog(zlog_cat[module], NULL, 0, NULL, 0, 0, bf_get_zlog_level(level),
          format, v);
    va_end(v);
  }
  return err;
}

int bf_sys_log_is_log_enabled(int module, int level) {
  if (module >= BF_MOD_MAX)
    return -1;
  if (module < 0)
    return -1;

  bool needs_trace = level <= bf_sys_trace_level[module];
  bool needs_log =
      zlog_would_log_at_level(zlog_cat[module], bf_get_zlog_level(level));

  if (needs_trace || needs_log)
    return 1;
  return 0;
}

/* pre defined log level names in zlog config file
 * *** IMPORTANT *** maintain the same index as BF_LOG_xxx
 * in bf_sys_log.h
 */
static const char zlog_cfg_level_name[BF_LOG_MAX + 1][8] = {
    "NONE", "FATAL", "ERROR", "WARN", "INFO", "DEBUG"};

int bf_sys_log_level_set(int module, int output, int bf_level) {
  char *sed_cmd = NULL;
  char mod_name[256];
  char level_name[8];
  int rc = 0, len = 0;

  /* we modify the current configuration file and reload it */

  /* sanity check */
  if (module >= BF_MOD_MAX || module < 0 || bf_level > BF_LOG_MAX ||
      bf_level < 0) {
    return -1;
  }
  /* support only stdout and file logging as output types */
  if (output != BF_LOG_DEST_STDOUT && output != BF_LOG_DEST_FILE) {
    return -1;
  }
  strncpy(mod_name, zlog_cat_name[module], sizeof(mod_name) - 1);
  strncpy(level_name, zlog_cfg_level_name[bf_level], sizeof(level_name) - 1);
  mod_name[sizeof(mod_name) - 1] = 0;
  level_name[sizeof(level_name) - 1] = 0;
  if (output == BF_LOG_DEST_STDOUT) {
    /* add a line in cfg file if it does not exist, else modify it */
    /* Set the length of the command string based on the length of each string
     * in the command plus a few more bytes (128) for the fixed part of the
     * command. */
    len = 128 + 4 * strlen(mod_name) + 3 * strlen(zlog_cur_log_file) +
          2 * strlen(level_name);
    sed_cmd = bf_sys_malloc(len);
    if (!sed_cmd)
      return -1;
    rc = snprintf(sed_cmd, len,
                  "grep -q \"%s.* >stdout\" %s && sed -i 's/%s.* >stdout/%s.%s "
                  ">stdout/' %s || echo '%s.%s >stdout;console_format' >> %s",
                  mod_name, zlog_cur_log_file, mod_name, mod_name, level_name,
                  zlog_cur_log_file, mod_name, level_name, zlog_cur_log_file);
  } else {
    /* file logging */
    /* Set the length of the command string based on the length of each string
     * in the command plus a few more bytes (64) for the fixed part of the
     * command. */
    len = 64 + 3 * strlen(mod_name) + 2 * strlen(zlog_cur_log_file) +
          strlen(level_name);
    sed_cmd = bf_sys_malloc(len);
    if (!sed_cmd)
      return -1;
    rc = snprintf(sed_cmd, len,
                  "grep -q \"%s.* \"\"\" %s && sed -i 's/%s.* \"/%s.%s \"/' %s",
                  mod_name, zlog_cur_log_file, mod_name, mod_name, level_name,
                  zlog_cur_log_file);
  }
  if (rc < 0 || rc >= len) {
    printf("Error setting log level: sts %d len %d module \"%s\" level \"%s\" "
           "file \"%s\"\n",
           rc, len, mod_name, level_name, zlog_cur_log_file);
    bf_sys_free(sed_cmd);
    return -1;
  }
  if (system(sed_cmd)) {
  }
  bf_sys_free(sed_cmd);
  return (bf_sys_log_zlog_reconfig(zlog_cur_log_file));
}

void bf_sys_trace_level_set(int module, int bf_level) {
  if (module >= BF_MOD_MAX) {
    return;
  }
  bf_sys_trace_level[module] = bf_level;
}

int bf_sys_trace_get(uint8_t *buf, size_t size, size_t *len_written) {
  memset(buf, 0, size); /* TBD: replace when implemented */
  *len_written = size;
  return 0;
}

int bf_sys_trace_reset(void) { return 0; }

int bf_sys_syslog_level_set(int bf_level) {
  char sed_cmd[100];
  char level_name[8];
  if (bf_level > BF_LOG_MAX) {
    return -1;
  }
  strncpy(level_name, zlog_cfg_level_name[bf_level], sizeof(level_name) - 1);
  level_name[sizeof(level_name) - 1] = 0;
  snprintf(sed_cmd, 100,
           "sed -i /syslog/d %s && echo '*.%s >syslog , LOG_USER' >> %s",
           zlog_cur_log_file, level_name, zlog_cur_log_file);
  if (system(sed_cmd) < 0) {
    return -1;
  }
  return (bf_sys_log_zlog_reconfig(zlog_cur_log_file));
}
