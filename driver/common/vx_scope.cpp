#include "vx_scope.h"
#include <VX_config.h>
#include <assert.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <scope-defs.h>
#include <thread>
#include <vector>
#include <vortex_afu.h>

#define FRAME_FLUSH_SIZE 100

#define CHECK_RES(_expr)                                                                  \
    do {                                                                                  \
        fpga_result res = _expr;                                                          \
        if (res == FPGA_OK)                                                               \
            break;                                                                        \
        printf("OPAE Error: '%s' returned %d, %s!\n", #_expr, (int)res, fpgaErrStr(res)); \
        return -1;                                                                        \
    } while (false)

#define MMIO_SCOPE_READ (AFU_IMAGE_MMIO_SCOPE_READ * 4)
#define MMIO_SCOPE_WRITE (AFU_IMAGE_MMIO_SCOPE_WRITE * 4)

#define CMD_GET_VALID 0
#define CMD_GET_DATA 1
#define CMD_GET_WIDTH 2
#define CMD_GET_COUNT 3
#define CMD_SET_START 4
#define CMD_SET_STOP 5
#define CMD_GET_OFFSET 6

static constexpr int num_modules = sizeof(scope_modules) / sizeof(scope_module_t);

static constexpr int num_taps = sizeof(scope_taps) / sizeof(scope_tap_t);

constexpr int calcFrameWidth(int index = 0) {
    return (index < num_taps) ? (scope_taps[index].width + calcFrameWidth(index + 1)) : 0;
}

static constexpr int fwidth = calcFrameWidth();

#ifdef HANG_TIMEOUT
static std::thread g_timeout_thread;
static std::mutex g_timeout_mutex;

static void timeout_callback(fpga_handle fpga) {
    std::this_thread::sleep_for(std::chrono::seconds{HANG_TIMEOUT});
    vx_scope_stop(fpga);
    fpgaClose(fpga);
    exit(0);
}
#endif

uint64_t print_clock(std::ofstream& ofs, uint64_t delta, uint64_t timestamp) {
    while (delta != 0) {
        ofs << '#' << timestamp++ << std::endl;
        ofs << "b0 0" << std::endl;
        ofs << '#' << timestamp++ << std::endl;
        ofs << "b1 0" << std::endl;
        --delta;
    }
    return timestamp;
}

void dump_taps(std::ofstream& ofs, int module) {
    for (int i = 0; i < num_taps; ++i) {
        auto& tap = scope_taps[i];
        if (tap.module != module)
            continue;
        ofs << "$var reg " << tap.width << " " << (i + 1) << " " << tap.name << " $end"
            << std::endl;
    }
}

void dump_module(std::ofstream& ofs, int parent) {
    for (auto& module : scope_modules) {
        if (module.parent != parent)
            continue;
        if (module.name[0] == '*') {
            ofs << "$var reg 1 0 clk $end" << std::endl;
        } else {
            ofs << "$scope module " << module.name << " $end" << std::endl;
        }
        dump_module(ofs, module.index);
        dump_taps(ofs, module.index);
        if (module.name[0] != '*') {
            ofs << "$upscope $end" << std::endl;
        }
    }
}

int vx_scope_start(fpga_handle hfpga, uint64_t start_time, uint64_t stop_time) {
    if (nullptr == hfpga)
        return -1;

    if (stop_time != uint64_t(-1)) {
        // set stop time
        uint64_t cmd_stop = ((stop_time << 3) | CMD_SET_STOP);
        CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, cmd_stop));
        std::cout << "scope stop time: " << std::dec << stop_time << "s" << std::endl;
    }

    // start recording
    uint64_t cmd_delay = ((start_time << 3) | CMD_SET_START);
    CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, cmd_delay));
    std::cout << "scope start time: " << std::dec << start_time << "s" << std::endl;

#ifdef HANG_TIMEOUT
    g_timeout_thread = std::thread(timeout_callback, hfpga);
    g_timeout_thread.detach();
#endif

    return 0;
}

int vx_scope_stop(fpga_handle hfpga) {
#ifdef HANG_TIMEOUT
    if (!g_timeout_mutex.try_lock())
        return 0;
#endif

    if (nullptr == hfpga)
        return -1;

    // forced stop
    uint64_t cmd_stop = ((0 << 3) | CMD_SET_STOP);
    CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, cmd_stop));

    std::cout << "scope trace dump begin..." << std::endl;

    std::ofstream ofs("trace.vcd");

    ofs << "$version Generated by Vortex Scope $end" << std::endl;
    ofs << "$timescale 1 ns $end" << std::endl;
    ofs << "$scope module TOP $end" << std::endl;

    dump_module(ofs, -1);
    dump_taps(ofs, -1);
    ofs << "$upscope $end" << std::endl;
    ofs << "enddefinitions $end" << std::endl;

    uint64_t frame_width, max_frames, data_valid, offset, delta;
    uint64_t timestamp    = 0;
    uint64_t frame_offset = 0;
    uint64_t frame_no     = 0;
    int signal_id         = 0;
    int signal_offset     = 0;

    // wait for recording to terminate
    CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, CMD_GET_VALID));
    do {
        CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &data_valid));
        if (data_valid)
            break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (true);

    // get frame width
    CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, CMD_GET_WIDTH));
    CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &frame_width));
    std::cout << "scope::frame_width=" << std::dec << frame_width << std::endl;

    if (fwidth != (int)frame_width) {
        std::cerr << "invalid frame_width: expecting " << std::dec << fwidth << "!" << std::endl;
        std::abort();
    }

    // get max frames
    CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, CMD_GET_COUNT));
    CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &max_frames));
    std::cout << "scope::max_frames=" << std::dec << max_frames << std::endl;

    // get offset
    CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, CMD_GET_OFFSET));
    CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &offset));

    // get data
    CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, CMD_GET_DATA));

    // print clock header
    CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &delta));
    timestamp = print_clock(ofs, offset + delta + 2, timestamp);
    signal_id = num_taps;

    std::vector<char> signal_data(frame_width + 1);

    do {
        if (frame_no == (max_frames - 1)) {
            // verify last frame is valid
            CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, CMD_GET_VALID));
            CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &data_valid));
            assert(data_valid == 1);
            CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, CMD_GET_DATA));
        }

        // read next data words
        uint64_t word;
        CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &word));

        do {
            int signal_width = scope_taps[signal_id - 1].width;
            int word_offset  = frame_offset % 64;

            signal_data[signal_width - signal_offset - 1] =
                ((word >> word_offset) & 0x1) ? '1' : '0';

            ++signal_offset;
            ++frame_offset;

            if (signal_offset == signal_width) {
                signal_data[signal_width] = 0;  // string null termination
                ofs << 'b' << signal_data.data() << ' ' << signal_id << std::endl;
                signal_offset = 0;
                --signal_id;
            }

            if (frame_offset == frame_width) {
                assert(0 == signal_offset);
                frame_offset = 0;
                ++frame_no;

                if (frame_no != max_frames) {
                    // print clock header
                    CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &delta));
                    timestamp = print_clock(ofs, delta + 1, timestamp);
                    signal_id = num_taps;
                    if (0 == (frame_no % FRAME_FLUSH_SIZE)) {
                        ofs << std::flush;
                        std::cout << "*** " << frame_no << "/" << max_frames << " frames"
                                  << std::endl;
                    }
                }
            }

        } while ((frame_offset % 64) != 0);

    } while (frame_no != max_frames);

    std::cout << "scope trace dump done! - " << (timestamp / 2) << " cycles" << std::endl;

    // verify data not valid
    CHECK_RES(fpgaWriteMMIO64(hfpga, 0, MMIO_SCOPE_WRITE, CMD_GET_VALID));
    CHECK_RES(fpgaReadMMIO64(hfpga, 0, MMIO_SCOPE_READ, &data_valid));
    assert(data_valid == 0);

    return 0;
}