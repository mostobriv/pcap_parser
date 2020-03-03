## Parameteres ##


# What boost::stacktrace should use
# Allowed values: BACKTRACE_SYSTEM, NONE
STACKTRACE_BACKEND = NONE


## Submodules and system dependecies ##


# This is used only so the libraries are built before the project itself is built,
# Remember to also add recipies for those
LIB_PREBUILT = lib/fmt/build/ \
               lib/PcapPlusPlus/mk/platform.mk \
               lib/inotify-cpp/build/src/libinotify-cpp.a

#submodule libraries
SUBMODULE_LIBS_DIR = -L ./lib/fmt/build \
                     -L ./lib/inotify-cpp/build/src \
                     -L ./lib/PcapPlusPlus/Dist
SUBMODULE_LIBS = -lfmt \
                 -linotify-cpp \
                 -lPcap++ \
                 -lPacket++ \
                 -lCommon++

#submodule include dirs
INCLUDES = -I ./lib/fmt/include \
           -I ./lib/inotify-cpp/src/include \
           -I ./lib/PcapPlusPlus/Dist/header

LIBRARIES = -lpcap -lpthread -lpqxx -lboost_system -lboost_filesystem -lstdc++fs
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_SYSTEM)
LIBRARIES := $(LIBRARIES) -ldl -lboost_stacktrace_backtrace
endif


## General build variables ##


SOURCES = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.h) $(wildcard src/*.hpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
BINARY  = parser

FLAGS = -g
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_SYSTEM)
FLAGS := $(FLAGS) -DBOOST_STACKTRACE_USE_BACKTRACE
else
FLAGS := $(FLAGS) -DBOOST_STACKTRACE_USE_NOOP
endif

WARNINGS = -Wall -Wextra -Werror -Wno-unused-parameter -Wno-deprecated-declarations


DEBUG = 


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


lib/fmt/build/:
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

lib/inotify-cpp/build/src/libinotify-cpp.a: lib/inotify-cpp/build
	@echo 'Building submodule inotify-cpp'
	@cd lib/inotify-cpp/build/ && cmake ..
	@$(MAKE) -C lib/inotify-cpp/build
lib/inotify-cpp/build:
	@echo 'Initializing submodule $@'
	@cd lib/inotify-cpp/ && git submodule update --init
	@mkdir lib/inotify-cpp/build
	#
	@echo 'Modifying inotify-cpp cmake file'
	@sed -i 's/\(add_library($${LIB_NAME} \)SHARED\( $${LIB_SRCS} $${LIB_HEADER})\)/\1STATIC\2/' lib/inotify-cpp/src/CMakeLists.txt
	@sed -i '/test/d; /example/d; /^$$/d'  lib/inotify-cpp/CMakeLists.txt


## Clean ##


clean:
	@rm -f objs/*
	@rm -f $(BINARY)


## Help ##


.PHONY: help
help:
	@echo '$(MAKE) [STACKTRACE_BACKEND=BACKEND] [TARGET]'
	@echo '    STACKTRACE_BACKEND - what to use for boost::stacktrace'
	@echo "        NONE (default) - don't use anything"
	@echo "        BACKTRACE_SYSTEM - use system gcc's backtrace.h"
	@echo
	@echo '    TARGET:'
	@echo '        clean'
	@echo '        all'
	@echo '        debug - all but with debug info'
	@echo '        $(BINARY) - main binary'


