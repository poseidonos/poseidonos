#!/bin/bash

./build_at_rsc_test.sh

taskset -c 9 ./measure_air_rsc.sh

cd ../../../../

make clean
