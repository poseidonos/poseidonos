AIR_TOP_DIR := $(abspath $(CURDIR)/../..)
GTEST_DIR := $(abspath $(CURDIR)/../../lib/googletest-master/googletest)
GMOCK_DIR := $(abspath $(CURDIR)/../../lib/googletest-master/googlemock)

CXX			= g++
CXXFLAGS	= -std=c++14 -fprofile-arcs -ftest-coverage -Wall
THREAD		= -pthread
TEST		= -lgtest
INCLUDES	= -I${AIR_TOP_DIR} -I${GTEST_DIR}/include -I${GMOCK_DIR}/include
TEST_SUITE	= test_suite
GCOV		= gcov

# add directory(module) which to unit-test here
TEST_DIRS = api chain collection config data_structure input lib meta object output policy process stream target_detector thread

test :
	@for dir in $(TEST_DIRS); do \
	$(MAKE) -C $$dir || exit $?; \
	done

clean :
	@for dir in $(TEST_DIRS); do \
	$(MAKE) clean -C $$dir || exit $?; \
	done

export AIR_TOP_DIR CXX CXXFLAGS THREAD TEST INCLUDES TEST_SUITE GCOV
