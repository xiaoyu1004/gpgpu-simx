#include "common.h"

#include <vx_intrinsics.h>
#include <vx_print.h>

void kernel_body() {
    // activate all threads
    vx_tmc(-1);

    kernel_arg_t* arg = (kernel_arg_t*)KERNEL_ARG_DEV_MEM_ADDR;
    uint32_t count    = arg->count;
    int32_t* src_ptr  = (int32_t*)arg->src_addr;
    int32_t* dst_ptr  = (int32_t*)arg->dst_addr;

    int num_cores  = vx_num_cores();
    int num_warps  = vx_num_warps();
    int num_theads = vx_num_threads();

    int core_id   = vx_core_id();
    int warp_id   = vx_warp_id();
    int thread_id = vx_thread_id();

    vx_printf("num_cores=%d, num_warps=%d, num_theads=%d, core_id=%d, warp_id=%d, thread_id=%d\n",
              num_cores,
              num_warps,
              num_theads,
              core_id,
              warp_id,
              thread_id);

    uint32_t offset = vx_core_id() * count;

    for (uint32_t i = 0; i < count; ++i) {
        dst_ptr[offset + i] = src_ptr[offset + i];
    }
}

void main() {
    vx_wspawn(4, kernel_body);

    kernel_body();

    // wait for all warps to complete
    vx_barrier(0, 4);

    // set warp0 to single-threaded and stop other warps
    int wid = vx_warp_id();
    vx_tmc(0 == wid);
}