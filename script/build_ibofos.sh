#!/bin/bash
#
# build_ibofos.sh
#

rootdir=$(readlink -f $(dirname $0))/..

cd $rootdir/lib
./build_ibof_lib.sh clean
./build_ibof_lib.sh all

cd $rootdir
make clean
make -j 8
