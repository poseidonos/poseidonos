#!/usr/bin/env python3

import common_test_lib

import os
import subprocess
import time
import psutil
import sys
import argparse

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path + "/../lib/")
import pos_util

#######################################################################################
# edit test parameters into these lists to run different workloads
transport = "tcp"
target_ip = "10.100.11.11"
ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"
script_path = ibof_root + "script/"
ibof_cli = ibof_root + "/bin/cli"
log_path = "pos.log"

nvme_device_0 = "unvme-ns-0"
nvme_device_1 = "unvme-ns-1"

volume_size = 2147483648*2
stdout_type = subprocess.DEVNULL
multi_threads = 24
print_on = False
clean=1


#######################################################################################

def execute_fio_pipe(readwrite, offset, verify="1"):
    print("\tExecute FIO")
    fio_bench = ibof_root + "test/system/io_path/fio_bench.py"
    process = subprocess.Popen([fio_bench,
                "--bs", "512B-128K", \
                "--readwrite", readwrite, \
                "-t", transport, \
                "-i", target_ip, \
                "--offset", str(offset), \
                "--io_size", str(100*1024*1024), \
                "--run_time", str("20"), \
                "--time_based", str("1"), \
                "--verify", verify, \
                "--file_num", str(multi_threads)]\
                )
    print("\tFIO start")
    return process

def clear_env():    
    pos_util.pci_rescan()
    common_test_lib.clear_env()
    subprocess.call("rm /dev/shm/* -rf", shell="True")

def detaching_device_during_io(read_or_write):

    for sleep_time in range(5,10):
        test_name = "Detaching During IO" + str(sleep_time)
        common_test_lib.print_start(test_name)
        common_test_lib.bringup_multiple_volume(**bringup_argument)
        verify_flag = "1"
        if("read" in read_or_write):
            verify_flag = "0"
        process = execute_fio_pipe(readwrite=read_or_write, offset=0, verify=verify_flag)

        time.sleep(7 + sleep_time)

        pos_util.pci_detach(nvme_device_0)

        print("Device Detached!!!!!")

        ret = process.wait()

        clear_env()
        common_test_lib.print_result(test_name, ret == 0)


def unmount_volume_during_io(read_or_write):

    for sleep_time in range(5,10):
        test_name = "Unmount During IO" + str(sleep_time)
        common_test_lib.print_start(test_name)
        common_test_lib.bringup_multiple_volume(**bringup_argument)
        verify_flag = "1"
        if("read" in read_or_write):
            verify_flag = "0"
        process = execute_fio_pipe(readwrite=read_or_write, offset=0, verify=verify_flag)

        time.sleep(7 + sleep_time)

        print("Volume Unmounted!!!!!")
        subprocess.call(ibof_cli + " volume unmount --name vol1 --array POSArray", shell="True")

        ret = process.wait()

        clear_env()
        common_test_lib.print_result(test_name, True)


def delete_volume_during_io(read_or_write):

    for sleep_time in range(5,10):
        test_name = "Unmount During IO" + str(sleep_time)
        common_test_lib.print_start(test_name)
        common_test_lib.bringup_multiple_volume(**bringup_argument)
        verify_flag = "1"
        if("read" in read_or_write):
            verify_flag = "0"
        process = execute_fio_pipe(readwrite=read_or_write, offset=0, verify=verify_flag)

        time.sleep(7 + sleep_time)

        print("Volume Unmounted!!!!!")
        subprocess.call(ibof_cli + " volume unmount --name vol1 --array POSArray", shell="True")
        print("Volume Deleted!!!!!")
        subprocess.call(ibof_cli + " volume delete --name vol1 --array POSArray", shell="True")

        ret = process.wait()

        clear_env()
        common_test_lib.print_result(test_name, True)


def stop_during_io(read_or_write):
    global bringup_argument
    for sleep_time in range(5, 10):
        test_name = "STOP During IO" + str(sleep_time)
        common_test_lib.print_start(test_name)
        common_test_lib.bringup_multiple_volume(**bringup_argument)

        process = execute_fio_pipe(readwrite=read_or_write, offset=0, verify="0")
        time.sleep(sleep_time)

        
        pos_util.pci_detach(nvme_device_0)
        print("Device Detached!!!!!")
        time.sleep(sleep_time * 0.1)

        pos_util.pci_detach(nvme_device_1)
        print("Other Device Detached!!!!!")

        ret = process.wait()

        clear_env()

        #fio can failed by floating point 0 issue. it is natural. 
        #if there is no hang issue, just pass.
        common_test_lib.print_result(test_name, True)


if __name__ == "__main__":    
    global bringup_argument
    parser = argparse.ArgumentParser(
            description='Please enter fabric ip options')
    parser.add_argument('-f', '--fabrics',\
            help='Specify ip address')
    parser.add_argument('-v', '--volume',\
            action='store_true', help='IO + Volume unmount or delete')
    args = parser.parse_args()
    if (args.fabrics != None):
        target_ip = args.fabrics

    bringup_argument = {
        'log_path' : log_path,
        'ibof_root' : ibof_root,
        'transport' : transport,
        'target_ip' : target_ip,
        'volume_size' : volume_size,
        'stdout_type' : stdout_type,
        'log_path' : log_path,
        'clean' : clean,
        'volume_cnt' : str(multi_threads),
        'subsystem_cnt' : str(multi_threads)
    }
    clear_env()
    if (args.volume == False):
        detaching_device_during_io("read")
        detaching_device_during_io("write")
        detaching_device_during_io("randwrite")
        stop_during_io("write")
        stop_during_io("read")
    else:
        unmount_volume_during_io("readwrite")
        delete_volume_during_io("readwrite")

