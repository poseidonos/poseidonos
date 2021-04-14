#!/bin/bash

./build_at_data_crd_test.sh

./test_scenarios.sh

cd ../../../../

make clean
