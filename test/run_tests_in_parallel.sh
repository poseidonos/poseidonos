#!/bin/bash

NUM_PROCS_UT=${NUM_PROCS_UT:-4}
NUM_PROCS_IT=${NUM_PROCS_IT:-1}

echo "Running unit tests in parallel... (NUM_PROCS = ${NUM_PROCS_UT})"
ctest -T Test -E "SystemTest\\\|IntegrationTest" -j ${NUM_PROCS_UT}

echo "Renaming unit test results"
find Testing/ -type f -name 'Test.xml' -exec mv {} {}.ut.xml \;

echo "Running integration tests in parallel... (NUM_PROCS = ${NUM_PROCS_IT})"
ctest -T Test -R "IntegrationTest" -j ${NUM_PROCS_IT}

echo "Renaming integration test results"
find Testing/ -type f -name 'Test.xml' -exec mv {} {}.it.xml \;
