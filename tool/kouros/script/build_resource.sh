#!/bin/bash

SCRIPT_PATH=$(readlink -f $(dirname $0))/

cd $SCRIPT_PATH

../bin/go-bindata -o ../utils/resource.go -pkg utils ../resources/
