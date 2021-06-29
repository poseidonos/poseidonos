#!/usr/bin/python3

import os
import subprocess
import argparse
import sys
import time

default_fabric_ip = "127.0.0.1"
ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"
mount_path = ibof_root + "test/system/filesystem/mnt"

def parse_arguments():
    parser = argparse.ArgumentParser(description='Filebench Initiator')
    parser.add_argument('-f', '--fabric_ip', default=default_fabric_ip,\
            help='Set fabric IP, default: ' + default_fabric_ip)
    global args
    args = parser.parse_args()
    print (args)

def connect():
    print ("Try to connect to target")
    subprocess.check_call(["nvme", "connect",
        "-t", "tcp",
        "-s", "1158",
        "-a", args.fabric_ip,
        "-n", "nqn.2019-04.pos:subsystem1"])

def get_target_device():
    print ("Try to get target device")
    command = "nvme list | awk '{if (0 != index($1,\"/dev/\")) dev=$1};END{print dev}'"
    global target_device
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE)
    target_device = result.stdout.decode('utf-8')[:-1]

    while target_device == "":
        print("Wait connect")
        result = subprocess.run(command, shell=True, stdout=subprocess.PIPE)
        target_device = result.stdout.decode('utf-8')[:-1]
        time.sleep(0.5)
    print ("Target Device is " + target_device)

def mount():
    print ("Try to make filesystem and mount")
    subprocess.check_call(["mkfs.xfs", "-f", target_device])
    subprocess.check_call(["rm", "-rf", mount_path])
    subprocess.check_call(["mkdir", mount_path])
    subprocess.check_call(["mount", target_device, mount_path])

def execute_filebench():
    execution_root = ibof_root + "test/system/filesystem"
    os.chdir(execution_root)
    subprocess.check_call(["filebench", "-f", "filebench_workload/fileserver.f"])
    subprocess.check_call(["filebench", "-f", "filebench_workload/oltp.f"])
    subprocess.check_call(["filebench", "-f", "filebench_workload/varmail.f"])
    subprocess.check_call(["filebench", "-f", "filebench_workload/videoserver.f"])
    subprocess.check_call(["filebench", "-f", "filebench_workload/webproxy.f"])
    subprocess.check_call(["filebench", "-f", "filebench_workload/webserver.f"])

def clean_up():
    subprocess.call(["umount", mount_path])
    subprocess.call(["nvme", "disconnect",
        "-n", "nqn.2019-04.pos:subsystem1"])
    subprocess.call("echo 0 > /proc/sys/kernel/randomize_va_space", shell=True)

if __name__ == "__main__":
    parse_arguments()
    clean_up()
    try:
        connect()
        get_target_device()
        mount()
        execute_filebench()
    except subprocess.CalledProcessError:
        print("Fail to execute command")
        clean_up()
        sys.exit(-1)

    clean_up()
    sys.exit(0)
