#!/usr/bin/env python3

import os
import subprocess
import time
import argparse
import sys
sys.path.append("../system/lib/")
import fileinput

import pos

default_fabric_ip = "10.1.11.200"
working_dir = os.path.dirname(os.path.realpath(__file__))
POS_ROOT = '../../'
test_suites = ["array", "devel", "device_management", "fault_tolerance", "logger", "simple_io", "subsystem", "system_overall", "volume"]


def run_tests(dirname):
    filenames = os.listdir(dirname)
    for filename in filenames:
        if filename.endswith(".py"):
            file_dir = working_dir + "/" + dirname
            print("********************** Testing " + filename + "*****************************")
            subprocess.call([working_dir+"/"+dirname+"/" +filename, args.fabric_ip], cwd=file_dir)
            time.sleep(30)
            os.system("rm -rf /dev/shm/ibof*")
            os.system("rm -rf /tmp/*.uram.*")
            os.system("rm -rf /var/tmp/spdk*")


def print_results(dirname):
    filenames = os.listdir(dirname)
    for filename in filenames:
        if filename.endswith(".result"):
            file_dir = working_dir + "/" + dirname
            print("********************** Testing " + filename + "*****************************")
            result = open(file_dir+"/"+filename)
            print(result.readline())


def mbr_reset():
    pos_mbr_reset = POS_ROOT + "test/script/mbr_reset.sh"
    subprocess.call([pos_mbr_reset])


def run_all_tests():
    for test_suite in test_suites:
        run_tests(test_suite)


def print_all_results():
    for test_suite in test_suites:
        print_results(test_suite)


def parse_argument():
    parser = argparse.ArgumentParser(description='SRM Test')
    parser.add_argument('-f', '--fabric_ip', default=default_fabric_ip,\
            help='Set target fabric IP, default: ' + default_fabric_ip)
    global args
    args = parser.parse_args()


if __name__ == "__main__":
    pos.enable_wt()
    mbr_reset()
    parse_argument()
    try:
        run_all_tests()
        print_all_results()

    except subprocess.CalledProcessError:
        print("Fail to execute command")
        pos.disable_wt()
        sys.exit(-1)
    except Exception as e:
        print("Fail to execute remote command", e)
        pos.disable_wt()
        sys.exit(-1)
    pos.disable_wt()
    sys.exit(0)
