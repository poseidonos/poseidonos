#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil
import sys
import paramiko
import time

default_fabric_ip = "127.0.0.1"
default_ibofos_root = "/home/ibof/ibofos"

def bring_up_ibofos():
    print ("Try to execute poseidonos at target")
    target_script = args.ibofos_root + "/test/system/filesystem/filebench_test_target.py -f " + args.fabric_ip
    result = subprocess.run(target_script, shell=True, stdout=subprocess.PIPE)
    print(result.stdout)

def execute_filebench_test():
    print ("Execute filebench at initiator")
    initiator_script = args.ibofos_root + "/test/system/filesystem/filebench_test_initiator.py -f " + args.fabric_ip
    result = subprocess.run(initiator_script, shell=True, stdout=subprocess.PIPE)
    print(result.stdout)
    
def parse_argument():
    parser = argparse.ArgumentParser(description='Filebench Test')
    parser.add_argument('-f', '--fabric_ip', default=default_fabric_ip,\
            help='Set target fabric IP, default: ' + default_fabric_ip)
    parser.add_argument('-r', '--ibofos_root', default=default_ibofos_root,\
            help='Set poseidonos root path, default: ' + default_ibofos_root)
    global args
    args = parser.parse_args()
    print (args)

def terminate_pos():
    unmount_array_command = args.ibofos_root + "/bin/poseidonos-cli array unmount --array-name POSArray --force"
    result = subprocess.run(unmount_array_command, shell=True, stdout=subprocess.PIPE)
    print(result.stdout)

    stop_pos_command = args.ibofos_root + "/bin/poseidonos-cli system stop --force"
    result = subprocess.run(stop_pos_command, shell=True, stdout=subprocess.PIPE)
    print(result.stdout)

    check_ibofos_command = "pgrep -c poseidonos"
    result = subprocess.run(check_ibofos_command, shell=True, stdout=subprocess.PIPE)
    print(result.stdout)


if __name__ == "__main__":
    parse_argument()
    try:
        bring_up_ibofos()
        execute_filebench_test()
        terminate_pos()

    except subprocess.CalledProcessError:
        print("Fail to execute command")
        terminate_pos()
        sys.exit(-1)
    except Exception as e:
        print("Fail to execute remote command", e)
        terminate_pos()
        sys.exit(-1)

    sys.exit(0)
