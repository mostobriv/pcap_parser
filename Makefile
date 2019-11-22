# include /usr/local/etc/PcapPlusPlus.mk

# this is used only for the libraries to be built before the project itself is built
LIB_INCLUDES = lib/fmt/include/ \
               lib/libbacktrace/libbacktrace/ \
               lib/PcapPlusPlus/Common++/
#submodule libraries
STATIC_LIBRARIES = lib/fmt/build/libfmt.a \
                   lib/libbacktrace/libbacktrace/libbacktrace.la \
                   lib/PcapPlusPlus/Common++/Lib/Release/libCommon++.a \
                   lib/PcapPlusPlus/Packet++/Lib/libPacket++.a \
                   lib/PcapPlusPlus/Pcap++/Lib/libPcap++.a
#submodule include dirs, also libpq
INCLUDES = -I /usr/include/postgresql \
           -I ./lib/fmt/include \
           -I ./lib/libbacktrace/libbacktrace \
           -I ./lib/PcapPlusPlus/Common++/header \
           -I ./lib/PcapPlusPlus/Packet++/header \
           -I ./lib/PcapPlusPlus/Pcap++/header

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
BINARY  = parser
LIBRARIES = -ldl -lboost_stacktrace_backtrace -lpq
FLAGS = -DBOOST_STACKTRACE_USE_BACKTRACE -g
DEBUG = 

.SILENT:

%.o: %.cpp | $(LIB_INCLUDES)
	@echo 'Building file: $<'
	@$(CXX) $(PCAPPP_BUILD_FLAGS) $(PCAPPP_INCLUDES) $(FLAGS) $(INCLUDES) $(DEBUG) -Wall -Wextra -std=c++17 -c -o $@ $<


all: $(BINARY)


debug: set-debug $(BINARY)


set-debug:
	$(eval DEBUG := -DDEBUG)


$(BINARY): $(OBJECTS) $(STATIC_LIBRARIES)
	@echo 'Building binary $@'
	@$(CXX) $(PCAPPP_LIBS_DIR) $(FLAGS) $(DEBUG) -o $@ $^ $(PCAPPP_LIBS) $(LIBRARIES)
	@$(PCAPPP_POST_BUILD)


lib/fmt/build/libfmt.a: | lib/fmt/build/
	@cd lib/fmt/build/ && cmake ..
	@$(MAKE) -C lib/fmt/build fmt
lib/fmt/build:
	@echo 'Initializing submodule $@'
	@cd lib/fmt && git submodule update --init
	@mkdir lib/fmt/build

lib/libbacktrace/libbacktrace/libbacktrace.la:
	@echo 'building libbacktrace'
	@cd lib/libbacktrace/libbacktrace/ && ./configure
	@$(MAKE) -C lib/libbacktrace/libbacktrace/
	#
	@echo 'Testing libbacktrace features'
	@cd lib/libbacktrace/ && $(CXX) supported.cpp && ./a.out

# as far as i know, there is no wasy to say that this recipie provides all three libs.
lib/PcapPlusPlus/Common++/Lib/Release/libCommon++.a: | lib/PcapPlusPlus/mk/
	@$(MAKE) -C lib/PcapPlusPlus/

lib/PcapPlusPlus/mk/:
	@echo 'Initializing submodule $@'
	@cd lib/PcapPlusPlus/ && git submodule update --init && ./configure-linux.sh --default


clean:
	@rm -f $(OBJECTS)
	@rm -f $(BINARY)
