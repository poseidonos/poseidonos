#!/bin/bash
#
# build_libair.sh
#

# Note: this is for making air library. before execute this, write -fPIC option in Makefile.
# libair usage: add build flag in your program build file. 
#               ex) LDFLAGS += -L./usr/local/lib/air -lair 

ROOTDIR=$(readlink -f $(dirname $0))/..

cd $ROOTDIR
make clean
./script/config.sh
make -j

rm -rf bin/obj/main.*
ar rscv libair.a bin/obj/*.o
mv libair.a /usr/local/lib/
cp src/api/air.h /usr/local/include
cp src/api/air_c.h /usr/local/include
