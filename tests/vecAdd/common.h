#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

#define KERNEL_ARG_DEV_MEM_ADDR 0x7ffff000

typedef struct {
    uint32_t count;
    uint32_t src_addr;
    uint32_t dst_addr;
} kernel_arg_t;

#endif