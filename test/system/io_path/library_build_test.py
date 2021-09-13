#!/usr/bin/env python3

import common_test_lib

import os
import subprocess
import argparse

#######################################################################################
# edit test parameters into these lists to run different workloads
ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"

#######################################################################################

stdout_type = subprocess.STDOUT
unittest_path = ["ubio_error_test", "mdts_detach_test"]

def build_ibofos_library_option():
    current_path = os.getcwd()
    os.chdir(ibof_root)

    subprocess.call(["./configure", \
        "--with-library-build"])
    
    ret = subprocess.call(["make", "-j4"])

    if(ret != 0):
        print("\tBuild Failed !!")
        exit(-1)

    os.chdir(current_path)        

def build_and_test(fabric_ip):
    current_path = os.getcwd()
    
    for test_path in unittest_path:
        test_path = current_path + "/" + test_path
        common_test_lib.print_start(test_path);
        os.chdir(test_path)
        
        ret = subprocess.call(["make"])            
        if(ret != 0):
            print("\tMake failed for %s" % (test_path))
            exit(-1)
        test_name = test_path.split('/')[-1]

        common_test_lib.kill_and_wait([test_name, "poseidonos", "fio"])

        ret = subprocess.call(["./" + test_name, "-a", fabric_ip])
        if (ret != 0 and ret != -9): #Sigkill is correct.
            print("\tTest failed for %s, ret : %d" % (test_path, ret))
            exit(-1)

    os.chdir(current_path)            
        
default_target_ip = "10.100.11.9"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='IO Unit Test')
    parser.add_argument('-f', '--fabric_ip', default=default_target_ip,\
            help='Set target IP, default: ' + default_target_ip)
    args = parser.parse_args()
    if(args.fabric_ip != None):
        default_target_ip = args.fabric_ip
    build_ibofos_library_option()
    print (default_target_ip)
    build_and_test(default_target_ip)
