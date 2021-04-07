#!/bin/bash
cd ../../
./configure --with-bdev-fio-plugin
export BUILD_IBOF_LIBRARY_BUILD=1
cd script/
./build_ibofos.sh
unset BUILD_IBOF_LIBRARY_BUILD
