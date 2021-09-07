#!/bin/bash
cd ../../../
make udev_uninstall
./lib/spdk/scripts/setup.sh reset
DEVICE_COUNT=4
END_INDEX=`expr $DEVICE_COUNT - 1`
for i in $(seq 0 $END_INDEX)
do
   nvme format /dev/nvme"$i"n1 -s 1 &
done
wait
./script/setup_env.sh
