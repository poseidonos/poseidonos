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
target_ip = "127.0.0.1"
pos_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"
script_path = pos_root + "script/"
pos_cli = pos_root + "/bin/poseidonos-cli"
log_path = "pos.log"

volume_size = 2147483648
stdout_type = sys.stdout
multi_threads = 2
print_on = False
clean = 1


#######################################################################################

def print_and_execute(cmd, value):
    print("")
    print(cmd)
    cmd += " --json-res | jq '.Response.result.status.code' 1>tmp_output"
    print("")
    ret = os.system(cmd)
    f = open("tmp_output")
    for line in f:
        ret_value = line.split('\n')[0]
        print("actual", ret_value, "expected", value)
        assert (ret_value == value)

def test_ok(vol, array, type, value):
    cmd = pos_cli + " qos list -a " + array + " -v " + vol
    cmd += " --json-res | jq '.Response.result.data.volumePolicies[0]." + type +"' 1>tmp_output"
    print("")
    print(cmd)
    print("")
    os.system(cmd)
    f = open("tmp_output")
    for line in f:
        ret_value = line.split('\n')[0]
        print("vol : ", vol, "array :", array, " ", type, " actual ", ret_value, "expected", value)
        assert (ret_value == value)

def test():
    os.system("rm /dev/shm/* -rf")
    common_test_lib.kill_and_wait(["poseidonos"])
    common_test_lib.bringup_multiple_volume(**bringup_argument)
    cmd = pos_cli + " qos create -a POSArray -v vol1 --minbw 0"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "minbw", "0")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --minbw 30"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "minbw", "30")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --maxbw 10"
    print_and_execute(cmd, "1858")
    test_ok("vol1", "POSArray", "maxbw", "0")
    test_ok("vol1", "POSArray", "minbw", "30")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --maxbw 50"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "minbw", "30")
    test_ok("vol1", "POSArray", "maxbw", "50")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --miniops 30"
    print_and_execute(cmd, "1858")
    test_ok("vol1", "POSArray", "minbw", "30")
    test_ok("vol1", "POSArray", "maxbw", "50")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --miniops 0"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "minbw", "30")
    test_ok("vol1", "POSArray", "maxbw", "50")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --minbw 0"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "minbw", "0")
    test_ok("vol1", "POSArray", "maxbw", "50")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --maxiops 70"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "minbw", "0")
    test_ok("vol1", "POSArray", "maxbw", "50")
    test_ok("vol1", "POSArray", "maxiops", "70")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --miniops 10"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "miniops", "10")
    test_ok("vol1", "POSArray", "maxbw", "50")
    test_ok("vol1", "POSArray", "maxiops", "70")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --miniops 13"
    print_and_execute(cmd, "1858")
    test_ok("vol1", "POSArray", "miniops", "10")
    test_ok("vol1", "POSArray", "maxbw", "50")
    test_ok("vol1", "POSArray", "maxiops", "70")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --maxbw 0"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "miniops", "10")
    test_ok("vol1", "POSArray", "maxbw", "0")
    test_ok("vol1", "POSArray", "maxiops", "70")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --miniops 50"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "miniops", "50")
    test_ok("vol1", "POSArray", "maxbw", "0")
    test_ok("vol1", "POSArray", "maxiops", "70")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --miniops 50"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "miniops", "50")
    test_ok("vol1", "POSArray", "maxbw", "0")
    test_ok("vol1", "POSArray", "maxiops", "70")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --miniops 20"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "miniops", "20")
    test_ok("vol1", "POSArray", "maxbw", "0")
    test_ok("vol1", "POSArray", "maxiops", "70")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --maxiops 10"
    print_and_execute(cmd, "1858")
    test_ok("vol1", "POSArray", "miniops", "20")
    test_ok("vol1", "POSArray", "maxbw", "0")
    test_ok("vol1", "POSArray", "maxiops", "70")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --miniops 0"
    print_and_execute(cmd, "0")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --maxiops 10"
    print_and_execute(cmd, "0")
    test_ok("vol1", "POSArray", "miniops", "0")
    test_ok("vol1", "POSArray", "maxbw", "0")
    test_ok("vol1", "POSArray", "maxiops", "10")
    cmd = pos_cli + " qos create -a POSArray -v vol1 --maxiops 8"
    print_and_execute(cmd, "1858")
    test_ok("vol1", "POSArray", "miniops", "0")
    test_ok("vol1", "POSArray", "maxbw", "0")
    test_ok("vol1", "POSArray", "maxiops", "10")

    cmd = pos_cli + " volume create -v vol3 --size " + str(volume_size) + "  --maxiops 5 --maxbw 5 --array-name POSArray"
    print_and_execute(cmd, "1858")

    cmd = pos_cli + " volume create -v vol3 --size " + str(volume_size) + "  --maxiops 444444444444444 --maxbw 444444444444444444 --array-name POSArray"
    print_and_execute(cmd, "1858")

    cmd = pos_cli + " volume create -v vol3 --size " + str(volume_size) + "  --maxiops 15 --maxbw 15 --array-name POSArray"
    print_and_execute(cmd, "0")
    test_ok("vol3", "POSArray", "maxbw", "15")
    test_ok("vol3", "POSArray", "maxiops", "15")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 100"
    print_and_execute(cmd, "0")
    test_ok("vol3", "POSArray", "maxbw", "15")
    test_ok("vol3", "POSArray", "maxiops", "100")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 0"
    print_and_execute(cmd, "0")
    test_ok("vol3", "POSArray", "maxbw", "15")
    test_ok("vol3", "POSArray", "maxiops", "0")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 0"
    print_and_execute(cmd, "0")
    test_ok("vol3", "POSArray", "maxbw", "15")
    test_ok("vol3", "POSArray", "maxiops", "0")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 100"
    print_and_execute(cmd, "0")
    test_ok("vol3", "POSArray", "maxbw", "15")
    test_ok("vol3", "POSArray", "maxiops", "100")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxbw 0 --minbw 380"
    print_and_execute(cmd, "0")
    test_ok("vol3", "POSArray", "minbw", "380")
    test_ok("vol3", "POSArray", "maxbw", "0")
    test_ok("vol3", "POSArray", "maxiops", "100")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxbw 0 --minbw 420"
    print_and_execute(cmd, "1858")
    test_ok("vol3", "POSArray", "minbw", "380")
    test_ok("vol3", "POSArray", "maxbw", "0")
    test_ok("vol3", "POSArray", "maxiops", "100")

    cmd = pos_cli + " volume create -v vol4 --size " + str(volume_size) + "  --maxiops 18446744073709550 --maxbw 15 --array-name POSArray"
    print_and_execute(cmd, "0")
    test_ok("vol4", "POSArray", "maxbw", "15")
    test_ok("vol4", "POSArray", "maxiops", "18446744073709550")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 18446744073709550 --minbw 0"
    print_and_execute(cmd, "0")
    test_ok("vol3", "POSArray", "minbw", "0")
    test_ok("vol3", "POSArray", "maxbw", "0")
    test_ok("vol3", "POSArray", "maxiops", "18446744073709550")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 18446744073709550 --maxbw 18446744073709550"
    print_and_execute(cmd, "1858")
    test_ok("vol3", "POSArray", "minbw", "0")
    test_ok("vol3", "POSArray", "maxbw", "0")
    test_ok("vol3", "POSArray", "maxiops", "18446744073709550")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 18446744073709550 --maxbw 17592186044415"
    print_and_execute(cmd, "0")
    test_ok("vol3", "POSArray", "minbw", "0")
    test_ok("vol3", "POSArray", "maxbw", "17592186044415")
    test_ok("vol3", "POSArray", "maxiops", "18446744073709550")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 18446744073709552 --maxbw 17592186044415"
    print_and_execute(cmd, "1858")
    test_ok("vol3", "POSArray", "minbw", "0")
    test_ok("vol3", "POSArray", "maxbw", "17592186044415")
    test_ok("vol3", "POSArray", "maxiops", "18446744073709550")

    cmd = pos_cli + " qos create -a POSArray -v vol3 --maxiops 18446744073709550 --maxbw 17592186044416"
    print_and_execute(cmd, "1858")
    test_ok("vol3", "POSArray", "minbw", "0")
    test_ok("vol3", "POSArray", "maxbw", "17592186044415")
    test_ok("vol3", "POSArray", "maxiops", "18446744073709550")

if __name__ == "__main__":
    global bringup_argument
    parser = argparse.ArgumentParser(
            description='Please enter fabric ip options')
    parser.add_argument('-f', '--fabrics',
                        help='Specify ip address')
    args = parser.parse_args()
    if (args.fabrics != None):
        target_ip = args.fabrics

    bringup_argument = {
        'log_path': log_path,
        'pos_root': pos_root,
        'transport': transport,
        'target_ip': target_ip,
        'volume_size': volume_size,
        'stdout_type': stdout_type,
        'log_path': log_path,
        'clean': clean,
        'volume_cnt': str(multi_threads),
        'subsystem_cnt': str(multi_threads),
        'stdout_type': sys.stdout
    }
    test()