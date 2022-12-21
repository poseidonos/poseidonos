#!/bin/bash

if [ $# -ne 2 ]
then
	echo -e "Syntax: \n	$0 [image_dir] [num_dev]\nExample:\n	$0 ~/workspace/qemu/vm1/devices 1"
	exit 1
fi

image_dir=$1
num_dev=$(($2 - 0))
tlc_size=$((5 * 1024 * 1024 * 1024))

mkdir -p $image_dir

if [ `ls -1 $image_dir/nvme.*.raw | awk 'END{print NR}'` -eq $(($num_dev + 1)) ]; then
	echo 'NVME device image(s) already exists.'
	exit 1
fi

for i in $(seq 0 $num_dev);
do
	echo Creating nvme device nvme.$i.raw;
	target_device=$image_dir/nvme.$i.raw
	truncate -s $tlc_size $target_device;
	count=$(expr $tlc_size / 10 / 1024 / 1024)
	dd if=/dev/zero of=$target_device bs=10M count=$count
done
