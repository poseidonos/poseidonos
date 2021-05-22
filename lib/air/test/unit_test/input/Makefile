SRCS += $(wildcard *.cpp)
MODULE_DIR = $(AIR_TOP_DIR)/src/input

all:
	$(CXX) $(CXXFLAGS) -o $(TEST_SUITE) $(SRCS) $(INCLUDES) $(THREAD) $(TEST)
	./$(TEST_SUITE) --gtest_output=xml
	$(GCOV) main.cpp > result.txt
	./gcov_module_result_parsing.sh $(MODULE_DIR)

clean:
	rm -rf $(TEST_SUITE)
	rm -rf *.xml
	rm -rf *.gcda *.gcno
	rm -rf result.txt
