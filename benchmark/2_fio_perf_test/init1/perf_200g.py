#!/usr/bin/env python3

import subprocess
import argparse
import os
import shutil

ibof_root = "/home/psd/poseidonos"
#os.path.dirname(os.path.abspath(__file__)) 
cur_dir = "."
TEST_NR=1

IOENGINE = ibof_root + "/lib/spdk/examples/nvme/fio_plugin/fio_plugin"
RUNTIME = "20"
IO_SIZE = "8g"
BLOCKSIZE = "128k"
NUMJOBS = "1"
RAMP_TIME = "5"
TIME_BASED = "1"
QD = "4"
IP="10.100.2.16"
TRANSPORT="tcp"
NUMCPUS="24"
RESULT=""
readwrite=["write","read","randwrite","randread"]

########################################################################################
def get_qd_and_filename():
    command=""
    for i in range(0, int(args.numcpus)):
       command += " --name=test" + str(i) + " --iodepth=" + str(args.qd) + " --filename='trtype=" + str(args.transport) + " adrfam=IPv4 traddr=" + str(args.address) + " trsvcid=1158 subnqn=nqn.2019-04.pos\:subsystem" + str(i*2+1) + " ns=1'"
    return command
            
def run_fio(workload, fio_command_for_each_test, fio_result_fd):
    command = "fio" \
            + " --name=global" \
            + " --ioengine=" + str(args.io_engine) + "" \
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
            + fio_command_for_each_test + "" 
    ret = subprocess.call(command,shell=True, stdout=fio_result_fd, stderr=fio_result_fd)
    return ret

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
    parser.add_argument('-w', '--wk', default=0,\
            help='Set wk, default: ' + "write")
    parser.add_argument('-q', '--qd', default=QD,\
            help='Set qd, default: ' + QD)
    parser.add_argument('-c', '--numcpus', default=NUMCPUS,\
            help='Set cpu, default: ' + NUMCPUS)
    parser.add_argument('-e', '--io_engine', default=IOENGINE,\
            help='Set io engine path, default: ' + IOENGINE)
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
    parser.add_argument('-i', '--address', default=IP,\
            help='Set IP address, default: ' + IP)
    parser.add_argument('-o', '--output', default=RESULT,\
            help='Set output file, default: ' + RESULT )
    global args
    args = parser.parse_args()

if __name__ == "__main__":
    parse_arguments()
    command =""
    LOG_DIR = cur_dir + "/" + args.blocksize + "_log"
    RESULT_FILE = LOG_DIR + "/" + args.blocksize + "_result"

    if os.path.isdir(LOG_DIR):
        shutil.rmtree(LOG_DIR)
    os.makedirs(LOG_DIR)

    success = True
    for i in range(0, TEST_NR):
        INDEXED_RESULT_FILE = RESULT_FILE + "_" + str(i)
        fio_result = open(INDEXED_RESULT_FILE, "w")
        fio_command_for_each_test = get_qd_and_filename()
        success &= run_fio(readwrite[int(args.wk)], fio_command_for_each_test, fio_result)
        fio_result.close()
        success &=check_fio_result(INDEXED_RESULT_FILE)
    success &=subprocess.call(cur_dir + "/parse_result.py " + RESULT_FILE + " " +  str(TEST_NR) + " > " + LOG_DIR + "/full_result", shell=True)
    if success != True:
        exit(1)
    else:
        exit(0)
