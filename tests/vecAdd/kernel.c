#include "common.h"

#include <vx_intrinsics.h>
#include <vx_print.h>

// #define DUMP_CSR_4(d, s)              \
//     csr_mem[d + 0] = csr_read(s + 0); \
//     csr_mem[d + 1] = csr_read(s + 1); \
//     csr_mem[d + 2] = csr_read(s + 2); \
//     csr_mem[d + 3] = csr_read(s + 3);

// #define DUMP_CSR_32(d, s)      \
//     DUMP_CSR_4(d + 0, s + 0)   \
//     DUMP_CSR_4(d + 4, s + 4)   \
//     DUMP_CSR_4(d + 8, s + 8)   \
//     DUMP_CSR_4(d + 12, s + 12) \
//     DUMP_CSR_4(d + 16, s + 16) \
//     DUMP_CSR_4(d + 20, s + 20) \
//     DUMP_CSR_4(d + 24, s + 24) \
//     DUMP_CSR_4(d + 28, s + 28)

void main() {
    // asm volatile(".insn s 0x6b, 1, %1, 0(%0)" ::"r"(4), "r"(STARTUP_ADDR + 16));

    // activate all threads
    vx_tmc(-1);

    kernel_arg_t* arg = (kernel_arg_t*)KERNEL_ARG_DEV_MEM_ADDR;
    uint32_t count    = arg->count;
    int32_t* src_ptr  = (int32_t*)arg->src_addr;
    int32_t* dst_ptr  = (int32_t*)arg->dst_addr;

    // int num_cores  = vx_num_cores();
    // int num_warps  = vx_num_warps();
    int num_theads = vx_num_threads();

    int core_id   = vx_core_id();
    int warp_id   = vx_warp_id();
    int thread_id = vx_thread_id();

    int idx = warp_id * num_theads + thread_id;

    dst_ptr[idx] = src_ptr[idx];

    // // set warp0 to single-threaded and stop other warps
    // vx_tmc(0 == warp_id);

    // uint32_t* const csr_mem = (uint32_t*)(IO_CSR_ADDR + 64 * sizeof(uint32_t) * core_id);
    // DUMP_CSR_32(0, CSR_MPM_BASE)
    // DUMP_CSR_32(32, CSR_MPM_BASE_H)

    // wait for all warps to complete
    vx_barrier(0, 4);

    vx_tmc(0);
}