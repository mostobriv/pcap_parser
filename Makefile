include /usr/local/etc/PcapPlusPlus.mk

SOURCES = $(wildcard src/*.cpp)
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES))
BINARY  = parser
LIBRARIES = -lfmt -ldl -lboost_stacktrace_backtrace -lpq
FLAGS = -DBOOST_STACKTRACE_USE_BACKTRACE -g
INCLUDES = -I/usr/include/postgresql
DEBUG = 

.SILENT:

%.o: %.cpp
	@echo 'Building file: $<'
	@$(CXX) $(PCAPPP_BUILD_FLAGS) $(PCAPPP_INCLUDES) $(FLAGS) $(INCLUDES) $(DEBUG) -Wall -std=c++2a -c -o $@ $<


all: $(BINARY)


debug: set_debug $(BINARY)


set_debug:
	$(eval DEBUG := -DDEBUG)


$(BINARY): $(OBJECTS)
	@$(CXX) $(PCAPPP_LIBS_DIR) $(FLAGS) $(DEBUG) -o $@ $^ $(PCAPPP_LIBS) $(LIBRARIES)
	@$(PCAPPP_POST_BUILD)


clean:
	rm $(OBJECTS) $(BINARY)