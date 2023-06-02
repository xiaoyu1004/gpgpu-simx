SIMX_DIR = ../simx

CXXFLAGS += -std=c++11 -Wall -Wextra -Wfatal-errors

CXXFLAGS += -fPIC -Wno-maybe-uninitialized
CXXFLAGS += -I./include -I./common -I../config -I$(SIMX_DIR) -I$(SIMX_DIR)/common

# CLUSTERS=1
# CORES=1
# WARPS=4
# THREADS=4
# L2=0
# L3=0
# PERF_FLAG=-DPERF_ENABLE
# CONFIGS=-DNUM_CLUSTERS=$(ERF_FLAG) -DNUM_CORES=$(CORES) -DNUM_WARPS=$(WARPS) -DNUM_THREADS=$(THREADS) -DL2_ENABLE=$(L2) -DL3_ENABLE=$(L3) $(PERF_FLAG)

# CXXFLAGS += $(CONFIGS)
CXXFLAGS += -DDUMP_PERF_STATS

LDFLAGS += -shared -pthread
LDFLAGS += -L$(SIMX_DIR) -lsimx

SRCS = vortex.cpp ./common/vx_utils.cpp

# Debugigng
ifdef DEBUG
	CXXFLAGS += -g -O0
else    
	CXXFLAGS += -O2 -DNDEBUG
endif

PROJECT = libvortex.so

all: $(PROJECT)

$(PROJECT): $(SRCS)
	DESTDIR=. $(MAKE) -C $(SIMX_DIR) libsimx.so
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

clean:
	DESTDIR=. $(MAKE) -C $(SIMX_DIR) clean
	rm -rf libsimx.so $(PROJECT) *.o