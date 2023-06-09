XLEN ?= 32

ifeq ($(XLEN),32)
RISCV_TOOLCHAIN_PATH = /opt/riscv-gnu-toolchain
else
RISCV_TOOLCHAIN_PATH = /opt/riscv64-gnu-toolchain
endif

RISCV_PREFIX ?= riscv$(XLEN)-unknown-elf-

CC = $(RISCV_TOOLCHAIN_PATH)/bin/$(RISCV_PREFIX)gcc
AR = $(RISCV_TOOLCHAIN_PATH)/bin/$(RISCV_PREFIX)gcc-ar
DP = $(RISCV_TOOLCHAIN_PATH)/bin/$(RISCV_PREFIX)objdump
CP = $(RISCV_TOOLCHAIN_PATH)/bin/$(RISCV_PREFIX)objcopy

ifeq ($(XLEN),32)
CFLAGS += -march=rv32imf -mabi=ilp32f
else
CFLAGS += -march=rv64imfd -mabi=lp64d
endif

CFLAGS += -O3 -mcmodel=medany -Wstack-usage=1024 -fno-exceptions -fdata-sections -ffunction-sections
CFLAGS += -I./include -I../config

PROJECT = libvortexrt

SRCS = ./src/vx_start.S ./src/vx_syscalls.c ./src/vx_print.S ./src/tinyprintf.c ./src/vx_print.c ./src/vx_spawn.c ./src/vx_spawn.S ./src/vx_perf.c

OBJS := $(addsuffix .o, $(notdir $(SRCS)))

all: $(PROJECT).a $(PROJECT).dump

$(PROJECT).dump: $(PROJECT).a
	$(DP) -D $(PROJECT).a > $(PROJECT).dump

%.S.o: src/%.S
	$(CC) $(CFLAGS) -c $< -o $@

%.c.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(PROJECT).a: $(OBJS)
	$(AR) rcs $@ $^

.depend: $(SRCS)
	$(CC) $(CFLAGS) -MM $^ > .depend;

clean:
	rm -rf *.a *.o *.dump .depend 