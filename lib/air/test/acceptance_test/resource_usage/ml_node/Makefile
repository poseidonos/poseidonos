TOP := $(abspath $(CURDIR)/../../../../)
USAGE_BINDIR = $(TOP)/test/acceptance_test/resource_usage/bin
DEFINE = -DAIR_CONFIG='"$(TOP)/config/"'

CXX         = g++
THREAD      = -lpthread
INCLUDE     = -I$(TOP) -I$(TOP)/include/
#DEBUG      = -g
LDFLAGS     += -L$(CURDIR)/../ -lair_ml
OBJ_FILES   = dummy_io.o main.o
TARGET      = ml_node

all: mkdir compile link clean

compile :
	$(CXX) $(DEBUG) $(INCLUDE) $(CXXFLAGS) $(DEFINE) -c main.cpp -o main.o
	$(CXX) $(DEBUG) $(INCLUDE) $(CXXFLAGS) $(DEFINE) -c dummy_io.cpp -o dummy_io.o

link :
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) \
	$(INCLUDE) $(LDFLAGS) -o $(USAGE_BINDIR)/$(TARGET)

mkdir :
	mkdir -p $(USAGE_BINDIR)

clean :
	rm -rf *.o
