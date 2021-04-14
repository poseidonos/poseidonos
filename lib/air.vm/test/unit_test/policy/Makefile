SRCS += $(wildcard *.cpp)
MODULE_DIR = $(AIR_TOP_DIR)/src/policy
DEFINE += -DAIR_CFG=config/mock_base.cfg

all:
	$(CXX) $(CXXFLAGS) $(DEFINE) -o $(TEST_SUITE) $(SRCS) $(INCLUDES) $(THREAD) $(TEST) $(LIB_DIRS)
	./$(TEST_SUITE) --gtest_output=xml
	$(GCOV) main.cpp > result.txt
	./gcov_module_result_parsing.sh $(MODULE_DIR)

clean:
	rm -rf $(TEST_SUITE)
	rm -rf *.xml
	rm -rf *.gcda *.gcno
	rm -rf result.txt
