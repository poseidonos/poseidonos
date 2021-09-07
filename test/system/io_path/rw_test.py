#!/usr/bin/env python3

import common_test_lib

import os
import subprocess
import argparse

#######################################################################################
# edit test parameters into these lists to run different workloads
pos_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"
default_transport = "tcp"
default_target_ip = "10.100.11.16"
script_path = pos_root + "script/"
default_log_path = "pos.log"
default_volume_size_gb = 16
default_print_on = False
#######################################################################################


def execute_fio(readwrite, offset, verify):
    print("\tExecute FIO")
    fio_bench = pos_root + "test/system/io_path/fio_bench.py"
    ret = subprocess.call([fio_bench,
                           "--bs", "4k",
                           "--readwrite", readwrite,
                           "-t", args.transport,
                           "-i", args.fabric_ip,
                           "--offset", str(offset),
                           "--io_size", "4k",
                           "--verify", str(verify)],
                          stdout=stdout_type, stderr=subprocess.STDOUT)
    print("\tFIO done")
    return ret


def write_test():
    test_name = "Write"
    common_test_lib.print_start(test_name)
    common_test_lib.bringup_pos(**bringup_argument)

    ret = execute_fio(readwrite="write", offset=0, verify=False)

    common_test_lib.terminate_pos(pos_root, stdout_type)
    success = (ret == 0)
    common_test_lib.print_result(test_name, success)
    return success


def write_out_range_fail_test():
    test_name = "Write out of range"
    common_test_lib.print_start(test_name)
    common_test_lib.bringup_pos(**bringup_argument)

    ret = execute_fio(readwrite="write", offset=volume_size, verify=False)

    common_test_lib.terminate_pos(pos_root, stdout_type)
    success = (ret == 1)
    common_test_lib.print_result(test_name, success)
    return success


def verify_test():
    test_name = "Verify"
    common_test_lib.print_start(test_name)
    common_test_lib.bringup_pos(**bringup_argument)

    ret = execute_fio(readwrite="write", offset=0, verify=True)

    common_test_lib.terminate_pos(pos_root, stdout_type)
    success = (ret == 0)
    common_test_lib.print_result(test_name, success)
    return success


def read_test():
    test_name = "Read"
    common_test_lib.print_start(test_name)
    common_test_lib.bringup_pos(**bringup_argument)

    ret = execute_fio(readwrite="read", offset=0, verify=False)

    common_test_lib.terminate_pos(pos_root, stdout_type)
    success = (ret == 0)
    common_test_lib.print_result(test_name, success)
    return success


def read_out_range_fail_test():
    test_name = "Read out of range"
    common_test_lib.print_start(test_name)
    common_test_lib.bringup_pos(**bringup_argument)

    ret = execute_fio(readwrite="read", offset=volume_size, verify=False)

    common_test_lib.terminate_pos(pos_root, stdout_type)
    success = (ret == 1)
    common_test_lib.print_result(test_name, success)
    return success


def parse_arguments():
    parser = argparse.ArgumentParser(description='Test normal read/write IO')
    parser.add_argument('-f', '--fabric_ip', default=default_target_ip,
                        help='Set target IP, default: ' + default_target_ip)
    parser.add_argument('-l', '--log_path', default=default_log_path,
                        help='Set path for log file, default: ' + default_log_path)
    parser.add_argument('-t', '--transport', default=default_transport,
                        help='Set transport, default: ' + default_transport)
    parser.add_argument('-s', '--volume_size', default=default_volume_size_gb,
                        help='Set volume size(GB), default: ' + str(default_volume_size_gb))
    parser.add_argument('-p', '--print_log', default=default_print_on,
                        help='Set printing log or not, default: ' + str(default_print_on))
    global args
    args = parser.parse_args()


def get_argument():
    gb_to_byte_shift = 30
    global volume_size
    volume_size = args.volume_size << gb_to_byte_shift
    global stdout_type
    if args.print_log:
        stdout_type = subprocess.STDOUT
    else:
        stdout_type = subprocess.DEVNULL

    bringup_argument = {
        'log_path': args.log_path,
        'pos_root': pos_root,
        'transport': args.transport,
        'target_ip': args.fabric_ip,
        'volume_size': volume_size,
        'stdout_type': stdout_type}
    print(args)
    return bringup_argument


if __name__ == "__main__":
    parse_arguments()
    bringup_argument = get_argument()
    common_test_lib.clear_env()
    success = write_test()
    success &= write_out_range_fail_test()
    success &= verify_test()
    success &= read_test()
    success &= read_out_range_fail_test()

    if (success):
        exit(0)
    else:
        exit(-1)
