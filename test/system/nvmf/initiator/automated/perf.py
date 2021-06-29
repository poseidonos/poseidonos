#!/usr/bin/env python3

import subprocess
import argparse
import os
import shutil
import sys

ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../../../"
cur_dir = ibof_root + "/test/system/nvmf/initiator/automated"
TEST_NR=3

IOENGINE = ibof_root + "/lib/spdk/build/fio/spdk_nvme"

RUNTIME = "10"
IO_SIZE = "8g"
BLOCKSIZE = "4k"
NUMJOBS = "1"
RAMP_TIME = "5"
TIME_BASED = "1"
IOAT_REACTOR_QD = "44,128"
NON_IOAT_REACTOR_QD = "128,128"
IP1="10.100.4.20"
IP2=""
TRANSPORT="tcp"

readwrite=["write","read","randwrite","randread"]
IOAT_CPULIST="0-7,15-22" # PSD: "0-7,23-30"
NON_IOAT_CPULIST= "8-14,23-30" # PSD: "8-22,31-46"

########################################################################################

def get_qd_of_workload():
    global rw_q
    rw_q=[]
    if "," in args.ioat_qd and "," in args.non_ioat_qd:
        ioat_wq, ioat_rq = args.ioat_qd.split(",",1)
        non_ioat_wq, non_ioat_rq = args.non_ioat_qd.split(",",1)
        rw_q=[[ioat_wq, non_ioat_wq],[ioat_rq, non_ioat_rq]]
    else:
        if "," in args.ioat_qd:
            ioat_wq, ioat_rq = args.ioat_qd.split(",",1)
            rw_q=[[ioat_wq, args.non_ioat_qd],[ioat_rq, arg.non_ioat_qd]]
        elif "," in args.non_ioat_qd:
            non_ioat_wq, non_ioat_rq = args.non_ioat_qd.split(",",1)
            rw_q=[[args.ioat_wq, non_ioat_qd],[args.ioat_rq, non_ioat_qd]]
        else:
            rw_q=[[args.ioat_qd, args.non_ioat_qd]]

def parse_input_cpulist():
    global ioat_cpulist
    ioat_cpulist = args.ioat_cpulist.split(',')
    global non_ioat_cpulist
    non_ioat_cpulist = args.non_ioat_cpulist.split(',')
    total_cpu = [ioat_cpulist, non_ioat_cpulist]

    return total_cpu

def get_ip_address(num):
    address = args.address
    if num % 2 == 1 and args.address2 != "":
        address = args.address2
    return address

def get_qd_and_filename(wk_qd):
    cpulist=[]
    ioat=[]
    non_ioat=[]
    total_cpu=[]
    total_cpu = parse_input_cpulist()
    for cpus in total_cpu:
        for value in cpus:   # cpus 는 ioat_cpulist & non_ioat_cpulist
            if "-" in value:
                x, y = value.split("-",1)
                for i in range(int(x), int(y)+1):
                    cpulist.append(i)
                    if cpus == ioat_cpulist:
                        ioat.append(i)
                    else:
                        non_ioat.append(i)
            else:
                cpulist.append(int(value))
                if cpus == ioat_cpulist:
                    ioat.append(value)
                else:
                    non_ioat.append(value)
    cpulist.sort()
    QD = ""
    command=""
    address=""
    global IP1, IP2
    for i in cpulist:
        if i in ioat:
            QD = wk_qd[0]
        else:
            QD = wk_qd[1]
        address = get_ip_address(i)
        command += " --name=test" + str(i) + " --iodepth=" + str(QD) + " --filename='trtype=" + str(args.transport) + " adrfam=IPv4 traddr=" + str(address) + " trsvcid=1158 subnqn=nqn.2019-04.pos\:subsystem" + str(i+1) + " ns=1'"
    return command
            
def run_fio(workload, fio_command_for_each_test, fio_result_fd):
    

    command = "fio" \
            + " --name=global" \
            + " --ioengine=" + str(IOENGINE) + "" \
            + " --runtime=" + str(args.runtime) + "" \
            + " --io_size=" + str(args.io_size) + "" \
            + " --readwrite=" + str(workload) + "" \
            + " --bs=" + str(args.blocksize) + "" \
            + " --thread=1" \
            + " --serialize_overlap=0" \
            + " --group_reporting=1" \
            + " --direct=1" \
            + " --numjobs=" + str(args.numjobs) + "" \
            + " --ramp_time=" + str(args.ramp_time) + "" \
            + " --time_based=" + str(args.time_based) + "" \
            + fio_command_for_each_test + "" \

    print (command)
    process = subprocess.Popen(command, shell=True,stdout=subprocess.PIPE, stderr=fio_result_fd)
    
    while process.poll() == None:
        out = process.stdout.readline()
        line = out.decode('utf-8') 
        fio_result_fd.write(line)
        print(line[:-1])

    return True

def check_fio_result(result_file_name):
    fio_result = open(result_file_name, "r")
    line = fio_result.read()
    fio_result.close()
    if "ERROR" in line or "failed" in line or "connection error" in line:
        print ("FIO Failed")
        return False
    return True

def parse_arguments():
    parser = argparse.ArgumentParser(description='Performance Test')
    parser.add_argument('-r', '--runtime', default=RUNTIME,\
            help='Set runtime, default: ' + RUNTIME)
    parser.add_argument('-b', '--blocksize', default=BLOCKSIZE,\
            help='Set blocksize, default: ' + BLOCKSIZE)
    parser.add_argument('-s', '--io_size', default=IO_SIZE,\
            help='Set io size, default: ' + IO_SIZE)
    parser.add_argument('-n', '--numjobs', default=NUMJOBS,\
            help='Set numjobs, default: ' + NUMJOBS)
    parser.add_argument('-a', '--ramp_time', default=RAMP_TIME,\
            help='Set ramp time, default: ' + RAMP_TIME)
    parser.add_argument('-t', '--time_based', default=TIME_BASED,\
            help='Set time based 1|0, default: ' + TIME_BASED)
    parser.add_argument('-p', '--transport', default=TRANSPORT,\
            help='Set transport tcp|rdma, default: ' + TRANSPORT)
    parser.add_argument('-i', '--address', default=IP1,\
            help='Set IP address1, default: ' + IP1)
    parser.add_argument('-i2', '--address2', default=IP2,\
            help='Set IP address2, default: ' + IP2)
    parser.add_argument('-x', '--ioat_qd', default=IOAT_REACTOR_QD,\
            help='Set ioat reactor qd for each write & read workload, default: ' + IOAT_REACTOR_QD)
    parser.add_argument('-y', '--non_ioat_qd', default=NON_IOAT_REACTOR_QD,\
            help='Set non ioat reactor qd for each write & rad workload, default: ' + NON_IOAT_REACTOR_QD)
    parser.add_argument('-w', '--ioat_cpulist', default=IOAT_CPULIST,\
            help='Set ioat cpulist, default: ' + IOAT_CPULIST)
    parser.add_argument('-z', '--non_ioat_cpulist', default=NON_IOAT_CPULIST,\
            help='Set non ioat cpulist, default: ' + NON_IOAT_CPULIST)
    global args
    args = parser.parse_args()

if __name__ == "__main__":
    parse_arguments()
    get_qd_of_workload()
    command =""
    
    LOG_DIR = cur_dir + "/" + args.blocksize + "_log"
    RESULT_FILE = LOG_DIR + "/" + args.blocksize + "_result"

    if os.path.isdir(LOG_DIR):
        shutil.rmtree(LOG_DIR)
    os.makedirs(LOG_DIR)

    success = True
    for i in range(0, TEST_NR): # total iteration
        INDEXED_RESULT_FILE = RESULT_FILE + "_" + str(i)
        fio_result = open(INDEXED_RESULT_FILE, "w")
        for i in range(len(rw_q)):
            fio_command_for_each_test = get_qd_and_filename(rw_q[i])
            for wk in readwrite:
                print(wk)
                success &= run_fio(wk, fio_command_for_each_test, fio_result)
        fio_result.close()
        success &=check_fio_result(INDEXED_RESULT_FILE)
    try:
        success &= subprocess.check_call(cur_dir+"/parse_result.py " + RESULT_FILE + " " +  str(TEST_NR) + " > " + LOG_DIR + "/full_result", shell=True) is 0
    except subprocess.CalledProcessError:
        print("Failed to Parse Result")
        sys.exit(1)
    if success == True:
        sys.exit(0)
    else:
        sys.exit(1)
