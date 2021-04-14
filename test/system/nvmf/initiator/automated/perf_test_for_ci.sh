#!/bin/bash

root_dir=$(readlink -f $(dirname $0))/../../../../..

target_fabric_ip="10.100.1.21"
ramp_time_param=0
file_num_param=8
verify_param=false
io_size_param=8g
block_size_param="4K,128K"
iodepth_param="1,4,32,128"
read_write_param="write,read,randwrite,randread"
cpus_allowed_param="2-25"
num_jobs_param=3

sudo $root_dir/test/system/nvmf/initiator/fio_full_bench.py \
		--traddr ${target_fabric_ip} --ramp_time=${ramp_time_param} \
		--file_num=${file_num_param} --verify=${verify_param} \
		--io_size=${io_size_param} --bs=${block_size_param} \
		--iodepth=${iodepth_param} --readwrite=${read_write_param} \
		--cpus_allowed=${cpus_allowed_param} --numjobs=${num_jobs_param}
