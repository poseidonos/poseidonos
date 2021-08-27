#!/bin/bash
#
# build_ibofos.sh
#

rootdir=$(readlink -f $(dirname $0))/..
BUILD_INTERNAL=FALSE

build_pos()
{
    cd $rootdir/lib
    if [ ${BUILD_INTERNAL} == TRUE ]; then
        cmake . -DSPDK_DEBUG_ENABLE=y -DUSE_LOCAL_REPO=y
        make clean_all
        make -j 4
    else
        cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=n
        make clean_all
        make -j 4
    fi
    cd $rootdir
    make clean
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