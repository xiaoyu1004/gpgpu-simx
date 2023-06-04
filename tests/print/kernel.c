#include <stdint.h>
#include <vx_intrinsics.h>
#include <vx_print.h>
#include <vx_spawn.h>

void kernel_body() {
    // activate all threads
    vx_tmc(-1);

    int core_id   = vx_core_id();
    int warp_id   = vx_warp_id();
    int thread_id = vx_thread_id();

    // vx_printf("core_id=%d, warp_id=%d, thread_id=%d\n", core_id, warp_id, thread_id);
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