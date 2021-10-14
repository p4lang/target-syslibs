project(libbf_sal VERSION 0.1 LANGUAGES C)

include_directories(${CMAKE_INSTALL_PREFIX}/include)
add_library(bf_sal_o OBJECT
bf_sys_ver.c
linux_usr/bf_sys_str.c
linux_usr/bf_sys_sal.c
linux_usr/bf_sys_mem.c
linux_usr/bf_sys_sem.c
linux_usr/bf_sys_timer.c
linux_usr/bf_sys_thread.c
linux_usr/bf_sys_log.c
linux_usr/bf_sys_log_internal.h
linux_usr/bf_sys_dma_hugepages.c)

target_compile_options(bf_sal_o PRIVATE  -Wno-pedantic)

if(TCMALLOC)
  add_dependencies(bf_sal_o tcmalloclib)
endif()

add_library(bf_sal SHARED EXCLUDE_FROM_ALL $<TARGET_OBJECTS:bf_sal_o>)
target_link_libraries(bf_sal PUBLIC ev zlog pthread)
