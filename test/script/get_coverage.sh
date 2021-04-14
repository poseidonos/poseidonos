#!/bin/bash

ROOT_DIR=../../

cd ${ROOT_DIR}
lcov -c -d src/ --rc lcov_branch_coverage=1 -o coverage.lcov
lcov --rc lcov_branch_coverage=1 -r coverage.lcov "/usr/*" "*/ibofos/lib/*" "*test*" "*wbt*" -o coverage.lcov
genhtml --branch-coverage -o coverage coverage.lcov
rm -rf coverage.lcov
tar -cvzf coverage.tar.gz coverage
rm -rf coverage
cd -
