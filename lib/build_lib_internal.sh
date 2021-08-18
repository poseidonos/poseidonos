#!/bin/bash

cmake . -DSPDK_DEBUG_ENABLE=y -DUSE_LOCAL_REPO=y
make -j 4