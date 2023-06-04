#include "common.h"

#include <vx_intrinsics.h>
#include <vx_print.h>

void main() {
    // activate all threads
    vx_tmc(-1);

    kernel_arg_t* arg = (kernel_arg_t*)KERNEL_ARG_DEV_MEM_ADDR;
    uint32_t count    = arg->count;
    int32_t* src_ptr  = (int32_t*)arg->src_addr;
    int32_t* dst_ptr  = (int32_t*)arg->dst_addr;

    // int num_cores  = vx_num_cores();
    // int num_warps  = vx_num_warps();
    int num_theads = vx_num_threads();

    // int core_id   = vx_core_id();
    int warp_id   = vx_warp_id();
    int thread_id = vx_thread_id();

    int idx = warp_id * num_theads + thread_id;

    dst_ptr[idx] = src_ptr[idx];

    // wait for all warps to complete
    vx_barrier(0, 4);

    // set warp0 to single-threaded and stop other warps
    // int wid = vx_warp_id();
    vx_tmc(0 == warp_id);

    vx_perf_dump();

    vx_tmc(0);
}