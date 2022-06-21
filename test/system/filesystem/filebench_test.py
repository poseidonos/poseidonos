#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil
import sys
import time

current_path = os.path.dirname(os.path.realpath(__file__))
lib_path = os.path.dirname(current_path) + "/lib"
sys.path.insert(1, lib_path)
import remote_procedure

default_fabric_ip = "127.0.0.1"
default_initiator_ip = "10.1.11.16"
default_target_ip = "10.1.11.16"
default_initiator_id = "root"
default_initiator_pw = "bamboo"
default_target_id = "root"
default_target_pw = "bamboo"
default_ibofos_root = "/home/ibof/ibofos"

def bring_up_ibofos():
    print("Try to execute poseidonos at target")
    target_script = args.ibofos_root + "/test/system/filesystem/filebench_test_target.py -f " + args.fabric_ip
    remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, target_script)

    time.sleep(10)
    print("Try to start telemetry through poseidonos-cli")
    target_script = args.ibofos_root + "/bin/poseidonos-cli telemetry start"
    remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, target_script)

def execute_filebench_test():
    print("Execute filebench at initiator")
    initiator_script = args.ibofos_root + "/test/system/filesystem/filebench_test_initiator.py -f " + args.fabric_ip
    remote_procedure.execute(args.initiator_ip, args.initiator_id, args.initiator_pw, initiator_script)

def parse_argument():
    parser = argparse.ArgumentParser(description='Filebench Test')
    parser.add_argument('-f', '--fabric_ip', default=default_fabric_ip,\
            help='Set target fabric IP, default: ' + default_fabric_ip)
    parser.add_argument('-i', '--initiator_ip', default=default_initiator_ip,\
            help='Set initiator IP, default: ' + default_initiator_ip)
    parser.add_argument('-t', '--target_ip', default=default_target_ip,\
            help='Set target IP, default: ' + default_target_ip)
    parser.add_argument('--target_pw', default=default_target_pw,\
            help='Set target PW, default: ' + default_target_pw)
    parser.add_argument('--initiator_pw', default=default_initiator_pw,\
            help='Set initiator PW, default: ' + default_initiator_pw)
    parser.add_argument('--target_id', default=default_target_id,\
            help='Set target ID, default: ' + default_target_id)
    parser.add_argument('--initiator_id', default=default_initiator_id,\
            help='Set initiator ID, default: ' + default_initiator_id)
    parser.add_argument('-r', '--ibofos_root', default=default_ibofos_root,\
            help='Set poseidonos root path, default: ' + default_ibofos_root)
    global args
    args = parser.parse_args()
    print (args)

def terminate_pos():
    unmount_array_command = args.ibofos_root + "/bin/poseidonos-cli array unmount --array-name POSArray --force"
    remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, unmount_array_command)
    stop_pos_command = args.ibofos_root + "/bin/poseidonos-cli system stop --force"
    remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, stop_pos_command)
    check_ibofos_command = "pgrep -c poseidonos"
    result = remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, check_ibofos_command)
    while (int(result) == 0):
        print("Wait exit")
        result = remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, check_ibofos_command)
        time.sleep(0.5)

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
