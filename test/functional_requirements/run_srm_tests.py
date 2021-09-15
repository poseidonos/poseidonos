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

def run_tests(dirname):
    filenames = os.listdir(dirname)
    for filename in filenames:
        if filename.endswith(".py"):
            file_dir = working_dir + "/" + dirname
            print("********************** Testing " + filename + "*****************************")
            subprocess.call([working_dir+"/"+dirname+"/" +filename, args.fabric_ip], cwd=file_dir)
            time.sleep(30)

def print_results(dirname):
    filenames = os.listdir(dirname)
    for filename in filenames:
        if filename.endswith(".result"):
            file_dir = working_dir + "/" + dirname
            print("********************** Testing " + filename + "*****************************")
            result = open(file_dir+"/"+filename)
            print(result.readline())

def run_all_tests():
    run_tests("array")
    run_tests("device_management")
    run_tests("fault_tolerance")
    run_tests("system_overall")
    run_tests("volume")

def print_all_results():
    print_results("array")
    print_results("device_management")
    print_results("fault_tolerance")
    print_results("system_overall")
    print_results("volume")

def parse_argument():
    parser = argparse.ArgumentParser(description='SRM Test')
    parser.add_argument('-f', '--fabric_ip', default=default_fabric_ip,\
            help='Set target fabric IP, default: ' + default_fabric_ip)
    global args
    args = parser.parse_args()

if __name__ == "__main__":
    parse_argument()
    try:
        run_all_tests()
        print_all_results()

    except subprocess.CalledProcessError:
        print("Fail to execute command")
        sys.exit(-1)
    except Exception as e:
        print("Fail to execute remote command", e)
        sys.exit(-1)
    sys.exit(0)
