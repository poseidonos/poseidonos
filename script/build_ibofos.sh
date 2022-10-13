#!/bin/bash
#
# build_ibofos.sh
#

source /etc/os-release

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

    if echo "$ID $VERSION_ID" | grep -E -q 'centos 8|rocky 8'; then
        export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/local/lib64/pkgconfig
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

