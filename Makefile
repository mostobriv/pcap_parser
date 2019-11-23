## Parameteres ##


# What boost::stacktrace should use
# Allowed values: BACKTRACE_SYSTEM, BACKTRACE_LIB, NONE
STACKTRACE_BACKEND = BACKTRACE_LIB


## Submodules and system dependecies ##


# This is used only so the libraries are built before the project itself is built,
# Remember to also add recipies for those
LIB_PREBUILT = lib/fmt/build/ \
               lib/PcapPlusPlus/mk/platform.mk
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
LIB_PREBUILT := $(LIB_PREBUILT) lib/libbacktrace/libbacktrace/libbacktrace.la
endif

#submodule libraries
STATIC_LIBRARIES = lib/fmt/build/libfmt.a \
                   lib/PcapPlusPlus/Common++/Lib/Release/libCommon++.a \
                   lib/PcapPlusPlus/Packet++/Lib/libPacket++.a \
                   lib/PcapPlusPlus/Pcap++/Lib/libPcap++.a
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
STATIC_LIBRARIES := $(STATIC_LIBRARIES) lib/libbacktrace/libbacktrace/.libs/libbacktrace.a
endif

#submodule include dirs, also libpq
INCLUDES = -I /usr/include/postgresql \
           -I ./lib/fmt/include \
           -I ./lib/PcapPlusPlus/Common++/header \
           -I ./lib/PcapPlusPlus/Packet++/header \
           -I ./lib/PcapPlusPlus/Pcap++/header
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
INCLUDES := $(INCLUDES) -I ./lib/libbacktrace/libbacktrace
endif

LIBRARIES = -lpq
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_SYSTEM)
LIBRARIES := $(LIBRARIES) -ldl -lboost_stacktrace_backtrace
endif


## General build variables ##


SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
BINARY  = parser

FLAGS = -g
ifeq ($(STACKTRACE_BACKEND),NONE)
FLAGS := $(FLAGS) -DBOOST_STACKTRACE_USE_NOOP
else
FLAGS := $(FLAGS) -DBOOST_STACKTRACE_USE_BACKTRACE
endif

WARNINGS = -Wall -Wextra -Werror -Wno-unused-parameter


DEBUG = 

.SILENT:


## General project files ##


%.o: %.cpp | $(LIB_PREBUILT)
	@echo 'Building file: $<'
	@$(CXX) $(PCAPPP_BUILD_FLAGS) $(PCAPPP_INCLUDES) $(FLAGS) $(INCLUDES) $(DEBUG) $(WARNINGS) -std=c++17 -c -o $@ $<


all: $(BINARY)


debug: set-debug $(BINARY)


set-debug:
	$(eval DEBUG := -DDEBUG)


$(BINARY): $(OBJECTS) $(STATIC_LIBRARIES)
	@echo 'Building binary $@'
	@$(CXX) $(PCAPPP_LIBS_DIR) $(FLAGS) $(DEBUG) -o $@ $^ $(PCAPPP_LIBS) $(LIBRARIES)
	@$(PCAPPP_POST_BUILD)


## Submodules ##


lib/fmt/build/libfmt.a: | lib/fmt/build/
	@echo 'Building submodule fmt'
	@cd lib/fmt/build/ && cmake ..
	@$(MAKE) -C lib/fmt/build fmt
lib/fmt/build/:
	@echo 'Initializing submodule $@'
	@cd lib/fmt && git submodule update --init
	@mkdir lib/fmt/build

ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
lib/libbacktrace/libbacktrace/libbacktrace.la:
	@echo 'building libbacktrace'
	@cd lib/libbacktrace/libbacktrace/ && ./configure
	@$(MAKE) -C lib/libbacktrace/libbacktrace/
	#
	@echo 'Testing libbacktrace features'
	@cd lib/libbacktrace/ && $(CXX) supported.cpp && ./a.out
endif

# as far as i know, there is no wasy to say that this recipie provides all three libs.
lib/PcapPlusPlus/Common++/Lib/Release/libCommon++.a: | lib/PcapPlusPlus/mk/
	@echo 'Building submodule PcapPlusPlus'
	@$(MAKE) -C lib/PcapPlusPlus/ libs
lib/PcapPlusPlus/mk/platform.mk:
	@echo 'Initializing submodule $@'
	@cd lib/PcapPlusPlus/ && git submodule update --init && ./configure-linux.sh --default


## Clean ##


clean:
	@rm -f $(OBJECTS)
	@rm -f $(BINARY)
