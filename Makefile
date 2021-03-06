## Parameteres ##


# What boost::stacktrace should use
# Allowed values: BACKTRACE_SYSTEM, NONE
STACKTRACE_BACKEND = NONE

DEBUG_INFO = FALSE

# Optimization level is unset for debug
ifeq ($(DEBUG_INFO),TRUE)
OPTIMIZATION = g
else
OPTIMIZATION = 0
endif


## Submodules and system dependecies ##


# This is used only so the libraries are built before the project itself is built,
# Remember to also add recipies for those
LIB_PREBUILT = lib/fmt/build/libfmt.a \
               lib/PcapPlusPlus/mk/platform.mk \
               lib/yaml-cpp/build/libyaml-cpp.a \
               lib/inotify-cpp/build/src/libinotify-cpp.a

#submodule libraries
SUBMODULE_LIBS_DIR = -L ./lib/fmt/build \
                     -L ./lib/inotify-cpp/build/src \
                     -L ./lib/yaml-cpp/build/ \
                     -L ./lib/PcapPlusPlus/Dist
SUBMODULE_LIBS = -lfmt \
                 -linotify-cpp \
                 -lyaml-cpp \
                 -lPcap++ \
                 -lPacket++ \
                 -lCommon++

#submodule include dirs
INCLUDES = -I ./lib/fmt/include \
           -I ./lib/inotify-cpp/src/include \
           -I ./lib/yaml-cpp/include \
           -I ./lib/PcapPlusPlus/Dist/header

LIBRARIES = -lpcap -lpthread -lpqxx -lstdc++fs
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_SYSTEM)
LIBRARIES := $(LIBRARIES) -ldl -lboost_stacktrace_backtrace
endif


## General build variables ##


SOURCES = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.h) $(wildcard src/*.hpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
BINARY  = parser


FLAGS = -O$(OPTIMIZATION)

ifeq ($(STACKTRACE_BACKEND),BACKTRACE_SYSTEM)
FLAGS := $(FLAGS) -DBOOST_STACKTRACE_USE_BACKTRACE
else
FLAGS := $(FLAGS) -DBOOST_STACKTRACE_USE_NOOP
endif

ifeq ($(DEBUG_INFO),TRUE)
FLAGS := $(FLAGS) -g
endif


WARNINGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-deprecated-declarations


## General project files ##


objs/%.o: src/%.cpp $(HEADERS) | $(LIB_PREBUILT) objs
	@echo 'Building file: $<'
	@$(CXX) $(PCAPPP_BUILD_FLAGS) $(PCAPPP_INCLUDES) $(FLAGS) $(INCLUDES) $(DEBUG) $(WARNINGS) -std=c++17 -c -o $@ $<


all: $(BINARY)


debug: set-debug $(BINARY)


set-debug:
	$(eval DEBUG := -DDEBUG)


$(BINARY): $(patsubst src/%.o,objs/%.o,$(OBJECTS))
	@echo 'Building binary $@'
	$(CXX) $(FLAGS) $(DEBUG) -o $@ $^ $(SUBMODULE_LIBS_DIR) $(SUBMODULE_LIBS) $(LIBRARIES)


## Directories ##


objs:
	mkdir objs


## Submodules ##


lib/fmt/build/libfmt.a:
	@echo 'Initializing submodule $@'
	@cd lib/fmt && git submodule update --init
	@mkdir lib/fmt/build
	#
	@echo 'Building submodule fmt'
	@cd lib/fmt/build/ && cmake ..
	@$(MAKE) -C lib/fmt/build fmt

lib/PcapPlusPlus/mk/platform.mk:
	@echo 'Initializing submodule $@'
	@cd lib/PcapPlusPlus/ && git submodule update --init && ./configure-linux.sh --default
	#
	@echo 'Building submodule PcapPlusPlus'
	@$(MAKE) -C lib/PcapPlusPlus/ libs

lib/inotify-cpp/build/src/libinotify-cpp.a:
	@echo 'Initializing submodule $@'
	@cd lib/inotify-cpp/ && git submodule update --init
	@mkdir lib/inotify-cpp/build
	#
	@echo 'Building submodule inotify-cpp'
	@cd lib/inotify-cpp/build/ && cmake ..
	@$(MAKE) -C lib/inotify-cpp/build

lib/yaml-cpp/build/libyaml-cpp.a:
	@echo 'Initializing submodule $@'
	@cd lib/yaml-cpp/ && git submodule update --init
	@mkdir lib/yaml-cpp/build
	#
	@echo 'Building submodule yaml-cpp'
	@cd lib/yaml-cpp/build/ && cmake .. -DYAML_CPP_BUILD_TESTS=OFF
	@$(MAKE) -C lib/yaml-cpp/build/


## Clean ##


clean:
	@rm -f objs/*
	@rm -f $(BINARY)


## Help ##


.PHONY: help
help:
	@echo '$(MAKE) [STACKTRACE_BACKEND=BACKEND] [OPTIMIZATION=LEVEL] [DEBUG_INFO=TRUE|FALSE] [TARGET]'
	@echo '    STACKTRACE_BACKEND - what to use for boost::stacktrace'
	@echo "        NONE (default) - don't use anything"
	@echo "        BACKTRACE_SYSTEM - use system gcc's backtrace.h"
	@echo
	@echo '    OPTIMIZATION - compiler optimization level'
	@echo '        Pass any option supported by your compiler -O option,'
	@echo '        e.g. 0, 3, fast'
	@echo
	@echo '    DEBUG_INFO - include debug info into binaries'
	@echo
	@echo '    TARGET:'
	@echo '        clean'
	@echo '        all'
	@echo '        debug - all but with debug info'
	@echo '        $(BINARY) - main binary'


