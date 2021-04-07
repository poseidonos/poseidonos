ARFLAGS = -rv

g_CPPFLAGS += -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include
g_CXXFLAGS += -g -Wall -Wextra -pthread -Wno-unused-parameter
ROOT := $(abspath $(CURDIR)/../../..)
INCLUDE = -I$(ROOT) -I$(ROOT)/lib

GTEST_VERSION = 1.10.x

GTEST_DIR := $(ROOT)/lib/googletest-release-$(GTEST_VERSION)/googletest
GMOCK_DIR := $(ROOT)/lib/googletest-release-$(GTEST_VERSION)/googlemock
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
				$(GTEST_DIR)/include/gtest/internal/*.h
GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
				$(GMOCK_DIR)/include/gmock/internal/*.h \
				$(GTEST_HEADERS)
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GMOCK_SRCS_ = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)

TARGET = array_ut

TEST_MODULES = array_fixture.cpp

MOCK_MODULES = 

OBJS = $(patsubst %.cpp, %.o, ../array.cpp) \
	   $(patsubst %.cpp, %.o, $(wildcard ../meta/*.cpp)) \
	   $(patsubst %.cpp, %.o, $(wildcard ../device/*.cpp)) \
	   $(patsubst %.cpp, %.o, $(wildcard ../partition/*.cpp)) \
	   $(patsubst %.cpp, %.o, $(wildcard ../ft/*.cpp)) \
	   $(patsubst %.cpp, %.o, $(wildcard ./fake/*.cpp)) \
	   $(patsubst %.cpp, %.o, $(ROOT)/src/device/ublock_device.cpp) \
	   $(patsubst %.cpp, %.o, $(ROOT)/src/master_context/mbr_manager_stub.cpp) \
	   $(patsubst %.cpp, %.o, $(ROOT)/src/device/device_manager_stub.cpp)

OBJ_DIR = ./objs
CXXFLAGS = -g -Wall -O0 -std=c++14

all : $(TARGET)

$(TARGET) : $(OBJS) $(TEST_MODULES) gmock_main.a objdir
	$(CXX) $(CXXFLAGS) $(INCLUDE) -pthread -o $@ gmock_main.a $(TEST_MODULES) $(addprefix $(OBJ_DIR)/, $(notdir $(OBJS)))

test : $(TEST_MODULES) gmock_main.a objdir
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $(TARGET) -pthread objs/*.o $^

$(OBJS) : %.o:%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c -o $(addprefix $(OBJ_DIR)/, $(notdir $@)) $<

objdir:
	@`[ -d $(OBJ_DIR) ] || mkdir -p $(OBJ_DIR)`

gmock_main.a : 
	$(CXX) $(g_CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(g_CXXFLAGS) \
		-c $(GTEST_DIR)/src/gtest-all.cc
	$(CXX) $(g_CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(g_CXXFLAGS) \
		-c $(GMOCK_DIR)/src/gmock-all.cc
	$(CXX) $(g_CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(g_CXXFLAGS) \
		-c $(GMOCK_DIR)/src/gmock_main.cc
	$(AR) $(ARFLAGS) $@ gmock-all.o gtest-all.o gmock_main.o
	rm -f gmock-all.o gtest-all.o gmock_main.o

clean :
	rm -f $(TARGET) objs/*.o gmock_main.a
