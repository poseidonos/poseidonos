#!/bin/bash
#
# build_ibofos.sh
#

rootdir=$(readlink -f $(dirname $0))/..
BUILD_INTERNAL=FALSE
BUILD_ASAN=n

build_pos()
{
    cd $rootdir/lib
    make clean_all
    if [ ${BUILD_INTERNAL} == TRUE ]; then
        cmake . -DSPDK_DEBUG_ENABLE=y -DUSE_LOCAL_REPO=y -DASAN_ENABLE=${BUILD_ASAN}
    else
        cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=n -DASAN_ENABLE=${BUILD_ASAN}
    fi
    make -j 4

    cd $rootdir
    make clean
    if [ ${BUILD_ASAN} == y ]; then
        ./configure --with-asan
    else
        ./configure --without-asan
    fi
    make -j 8
}

while getopts "i" opt
do
    case "$opt" in
        i) BUILD_INTERNAL=TRUE
            ;;
    esac
done

build_pos