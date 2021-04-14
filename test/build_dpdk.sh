#!/bin/bash

echo changing to {ibof_home}/lib/dpdk-20.08
cd ../lib/dpdk-20.08

echo meson build
meson build

#Using meson is recommended way, but I will keep this block commented out for now since our main build uses "Makefile" instead of "meson install"
#echo changing to {ibof_home}/lib/dpdk/build
#meson install

echo changing to {ibof_home}/lib 
cd ..
./build_ibof_lib.sh dpdk

