#!/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/../../
sudo mkdir -p /etc/pos/
sudo cp ./pos.conf /etc/pos/
sudo $ROOT_DIR/script/setup_env.sh
sudo $ROOT_DIR/bin/poseidonos > /dev/null &
