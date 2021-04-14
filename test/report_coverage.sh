#!/bin/bash

rm -f coverage.info
rm -f coverage_filtered.lcov

lcov --rc lcov_branch_coverage=1 --capture --directory . --output-file coverage.info
lcov --rc lcov_branch_coverage=1 -r coverage.info '/usr/*' '*/unit-tests/*' '*/integration-tests/*' '*/spdlog-1.4.2/*' -o coverage_filtered.lcov
mkdir -p build
genhtml coverage_filtered.lcov --output-directory build/coverage-html  --rc lcov_branch_coverage=1 