#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil
import sys
import paramiko
import time

default_fabric_ip = "127.0.0.1"
default_initiator_ip = "10.1.11.16"
default_target_ip = "10.1.11.16"
default_initiator_id = "root"
default_initiator_pw = "bamboo"
default_target_id = "root"
default_target_pw = "bamboo"
default_ibofos_root = "/home/ibof/ibofos"

def remote_execute(ip, id, pw, command):
    cli = paramiko.SSHClient()
    cli.load_system_host_keys()
    cli.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    cli.connect(ip, port=22, username=id, password=pw)
    stdin, stdout, stderr = cli.exec_command(command)
    result=""

    for line in iter(stdout.readline, ""):
       print(line, end="")
       result += line
    cli.close()
    return result

def bring_up_ibofos():
    print ("Try to execute ibofos at target")
    target_script = args.ibofos_root + "/test/system/filesystem/filebench_test_target.py -f " + args.fabric_ip
    remote_execute(args.target_ip, args.target_id, args.target_pw, target_script)

def execute_filebench_test():
    print ("Execute filebench at initiator")
    initiator_script = args.ibofos_root + "/test/system/filesystem/filebench_test_initiator.py -f " + args.fabric_ip
    remote_execute(args.initiator_ip, args.initiator_id, args.initiator_pw, initiator_script)

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
            help='Set ibofos root path, default: ' + default_ibofos_root)
    global args
    args = parser.parse_args()
    print (args)

def terminate_ibofos():
    unmount_ibofos_command = args.ibofos_root + "/bin/cli request unmount_ibofos"
    remote_execute(args.target_ip, args.target_id, args.target_pw, unmount_ibofos_command)
    exit_ibofos_command = args.ibofos_root + "/bin/cli request exit_ibofos"
    remote_execute(args.target_ip, args.target_id, args.target_pw, exit_ibofos_command)
    check_ibofos_command = "pgrep -c ibofos"
    result = remote_execute(args.target_ip, args.target_id, args.target_pw, check_ibofos_command)
    while (int(result) == 0):
        print("Wait exit")
        result = remote_execute(args.target_ip, args.target_id, args.target_pw, check_ibofos_command)
        time.sleep(0.5)

if __name__ == "__main__":
    parse_argument()
    try:
        bring_up_ibofos()
        execute_filebench_test()
        terminate_ibofos()

    except subprocess.CalledProcessError:
        print("Fail to execute command")
        terminate_ibofos()
        sys.exit(-1)

    sys.exit(0)
