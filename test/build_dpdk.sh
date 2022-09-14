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

case "$1" in
internal)
    cmake . -DSPDK_DEBUG_ENABLE=y -DUSE_LOCAL_REPO=y -DASAN_ENABLE=n
    make -j 4 dpdk
    ;;
*)
    cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=n -DASAN_ENABLE=n
    make -j 4 dpdk
    ;;
esac

