#!/bin/bash

rootdir=$(readlink -f $(dirname $0))/..
cd $rootdir/lib
cmake . -DSPDK_DEBUG_ENABLE=y -DUSE_LOCAL_REPO=y -DASAN_ENABLE=n
make -j 4
