#!/bin/bash
#
# build_ibofos.sh
#

rootdir=$(readlink -f $(dirname $0))/..

cd $rootdir/lib
case "$1" in
internal)
    cmake . -DSPDK_DEBUG_ENABLE=y -DUSE_LOCAL_REPO=y
    make clean_all
    make -j 4
    ;;
*)
    cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=n
    make clean_all
    make -j 4
    ;;
esac

cd $rootdir
make clean
make -j 8
