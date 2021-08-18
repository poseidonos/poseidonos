#!/bin/bash

cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=n
make -j 4