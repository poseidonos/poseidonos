#!/bin/bash

rootdir=$(readlink -f $(dirname $0))/..
cd $rootdir/lib
cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=n
make -j 4
