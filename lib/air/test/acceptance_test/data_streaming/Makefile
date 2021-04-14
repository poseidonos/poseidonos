TOP := $(abspath $(CURDIR)/../../../)

DEFINE += -DAIR_CONFIG='"$(TOP)/config/"'

CXX         = g++
CXXFLAGS    += -std=c++17 -Wall -O2 -fPIC
THREAD      = -lpthread
INCLUDE     = -I$(TOP) -I$(TOP)/include/
#DEBUG		= -g
LDFLAGS     += -L$(TOP)/lib -lair
OBJ_FILES	+= result_handler.o rule_checker.o dummy_io.o main.o
TARGET		= data_stream

all : test_code link

test_code :
	$(CXX) $(DEBUG) $(INCLUDE) $(CXXFLAGS) $(DEFINE) -c result_handler.cpp -o result_handler.o
	$(CXX) $(DEBUG) $(INCLUDE) $(CXXFLAGS) $(DEFINE) -c rule_checker.cpp -o rule_checker.o
	$(CXX) $(DEBUG) $(INCLUDE) $(CXXFLAGS) $(DEFINE) -c dummy_io.cpp -o dummy_io.o
	$(CXX) $(DEBUG) $(INCLUDE) $(CXXFLAGS) $(DEFINE) -c main.cpp -o main.o

link :
	$(CXX) $(DEBUG) $(OBJ_FILES) $(CXXFLAGS) $(THREAD) $(DEFINE) \
	$(INCLUDE) $(LDFLAGS) -o $(TARGET)

clean :
	rm -rf *.o $(TARGET)
	rm -rf *.xml
