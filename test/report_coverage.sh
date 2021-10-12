#!/bin/bash

# Clean up previously-generated profiled data
rm -f coverage.info
rm -f coverage_filtered.lcov
rm -f coverage_filtered_pos.lcov

# Produce 'coverage.info'
lcov --rc lcov_branch_coverage=1 --capture --directory . --output-file coverage.info

# Produce 'coverage_filtered.lcov'
lcov --rc lcov_branch_coverage=1 -r coverage.info '/usr/*' '*/unit-tests/*' '*/integration-tests/*' '*/spdlog-1.4.2/*' '*/include/nlohmann/json.hpp' '*/test/utils/*' '*/air/src/*' '*/generated/*' -o coverage_filtered.lcov

# Run the UT of the script first
python filter_out_wrong_branches_test.py

# Produce 'coverage_filtered_pos.lcov'
python filter_out_wrong_branches.py coverage_filtered.lcov

# Produce test coverage report at build/coverage-html
mkdir -p build
genhtml coverage_filtered_pos.lcov --output-directory build/coverage-html  --rc lcov_branch_coverage=1 --demangle-cpp --legend
