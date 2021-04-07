SRCS += $(wildcard *.cpp)
MODULE_DIR = $(AIR_TOP_DIR)/src/config
DEFINE += -DAIR_CFG=config/ut_config.cfg

all:
	$(CXX) -g $(CXXFLAGS) $(DEFINE) -o $(TEST_SUITE) $(SRCS) $(INCLUDES) $(THREAD) $(TEST) $(LIB_DIRS)
	./$(TEST_SUITE) --gtest_output=xml
	$(GCOV) main.cpp > result.txt
	./gcov_module_result_parsing.sh $(MODULE_DIR)
	rm -rf $(TEST_SUITE)

clean:
	rm -rf $(TEST_SUITE)
	rm -rf *.xml
	rm -rf *.gcda *.gcno
	rm -rf result.txt

