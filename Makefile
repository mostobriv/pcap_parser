## Parameteres ##


# What boost::stacktrace should use
# Allowed values: BACKTRACE_SYSTEM, BACKTRACE_LIB, NONE
STACKTRACE_BACKEND = NONE


## Submodules and system dependecies ##


# This is used only so the libraries are built before the project itself is built,
# Remember to also add recipies for those
LIB_PREBUILT = lib/fmt/build/ \
               lib/PcapPlusPlus/mk/platform.mk
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
LIB_PREBUILT := $(LIB_PREBUILT) lib/libbacktrace/libbacktrace/libbacktrace.la
endif

#submodule libraries
SUBMODULE_LIBS_DIR = -L ./lib/fmt/build \
		     -L ./lib/PcapPlusPlus/Dist
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
SUBMODULE_LIBS_DIR := $(SUBMODULE_LIBS_DIR) \
	-L ./lib/libbacktrace/libbacktrace/.libs
endif
SUBMODULE_LIBS = -lfmt \
                 -lPcap++ \
                 -lPacket++ \
                 -lCommon++
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
SUBMODULE_LIBS := $(SUBMODULE_LIBS) -lbacktrace
endif

#submodule include dirs
INCLUDES = -I ./lib/fmt/include \
	   -I lib/PcapPlusPlus/Dist/header
ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
INCLUDES := $(INCLUDES) -I ./lib/libbacktrace/libbacktrace
endif

LIBRARIES = -lpcap -lpthread
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


## General project files ##


%.o: %.cpp | $(LIB_PREBUILT)
	@echo 'Building file: $<'
	@$(CXX) $(PCAPPP_BUILD_FLAGS) $(PCAPPP_INCLUDES) $(FLAGS) $(INCLUDES) $(DEBUG) $(WARNINGS) -std=c++17 -c -o $@ $<


all: $(BINARY)


debug: set-debug $(BINARY)


set-debug:
	$(eval DEBUG := -DDEBUG)


$(BINARY): $(OBJECTS)
	@echo 'Building binary $@'
	$(CXX) $(FLAGS) $(DEBUG) -o $@ $^ $(SUBMODULE_LIBS_DIR) $(LIBRARIES) $(SUBMODULE_LIBS)


## Submodules ##


lib/fmt/build/:
	@echo 'Initializing submodule $@'
	@cd lib/fmt && git submodule update --init
	@mkdir lib/fmt/build
	#
	@echo 'Building submodule fmt'
	@cd lib/fmt/build/ && cmake ..
	@$(MAKE) -C lib/fmt/build fmt

ifeq ($(STACKTRACE_BACKEND),BACKTRACE_LIB)
lib/libbacktrace/libbacktrace/libbacktrace.la:
	@echo 'building libbacktrace'
	@cd lib/libbacktrace/libbacktrace/ && ./configure
	@$(MAKE) -C lib/libbacktrace/libbacktrace/
	#
	@echo 'Testing libbacktrace features'
	@cd lib/libbacktrace/ && $(CXX) supported.cpp && ./a.out
endif

lib/PcapPlusPlus/mk/platform.mk:
	@echo 'Initializing submodule $@'
	@cd lib/PcapPlusPlus/ && git submodule update --init && ./configure-linux.sh --default
	#
	@echo 'Building submodule PcapPlusPlus'
	@$(MAKE) -C lib/PcapPlusPlus/ libs


## Clean ##


clean:
	@rm -f $(OBJECTS)
	@rm -f $(BINARY)
