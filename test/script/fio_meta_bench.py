#!/usr/bin/env python3

import subprocess
from subprocess import check_call, call, check_output, Popen, PIPE
import random
import os
import sys
import re
import signal
import getopt
from datetime import datetime
from itertools import *
import csv
import itertools
from shutil import copyfile
import json
import optparse

#######################################################################################
# edit test parameters into these lists to run different workloads
ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../../"
ioengine = ibof_root + "lib/spdk/examples/nvme/fio_plugin/fio_plugin"

# 1 namespace
traddr=""
trtype="tcp"

# the configuration below runs QD 1 & 128. 
# To add set q_depth=['1', '32', '128']
q_depth=['128']

# io_size and block_size specifies the size in bytes of the IO workload.
# To add 64K IOs set io_size = ['4096', '65536'] and block_size = [ '512', '1024', '4096', '512-128k' ]
io_size_bytes=['20m']

block_size=['4096']

# type of I/O pattern : write, read, trim, randread, randwrite, randtrim, readwrite, rw, randrw
readwrite=['write']

# verify = True | False. applied on sorts of write I/O patterns
verify=False

# run_time parameter specifies how long to run each test.
# Set run_time = ['10', '600'] to run the test for given seconds
run_time=['5']

# mixed rw ratio
mix=['100']

# cpu affinity to run fio (comma-delimited)
cpus_allowed=['']

# iter_num parameter is used to run the test multiple times.
# set iter_num = ['1', '2', '3'] to repeat each test 3 times
iter_num=['1']

# setting profile_mode True | False. True will remains profile json file and .csv result file
profile_mode=False

# verbose = True | False. setting True will show more fio log
verbose=True

numjobs='1'
time_based='1'
ramp_time='2'
file_num=1

# extra fio options
extra_fio_options="--thread=1 --group_reporting=1 --direct=1"

#######################################################################################

red = "\033[1;31m"
green = "\033[0;32m"
reset = "\033[0;0m"

def run_fio(io_size_bytes, block_size, qd, rw_mix, cpus_allowed, run_num, workload, run_time_sec):
    sys.stdout.write(red)
    print("[TEST {}] ".format(run_num), end='')
    sys.stdout.write(green)
    print("Started. size={} block_size={} qd={} io_pattern={} mix={} cpus_allowed={} time={}".format(io_size_bytes, block_size, qd, workload, rw_mix, cpus_allowed, run_time_sec))
    sys.stdout.write(reset)
    print("")

    # call fio

    command = "fio "\
            + " --ioengine=" + str(ioengine) + "" \
            + " --runtime=" + str(run_time_sec) + "" \
            + " --io_size=" + str(io_size_bytes) + "" \
            + " --iodepth=" + str(qd) + "" \
            + " --readwrite=" + str(workload) + ""

    if cpus_allowed != "":
        command += " --cpus_allowed=" + str(cpus_allowed) + ""

    if block_size.find('-') == -1:
        command += " --bs_unaligned=1 --bs=" + str(block_size)
    else:
        command += " --bsrange=" + str(block_size)

    if workload.find('rw') == -1:
        command += " --norandommap=1 "

    if verify == True:
        command += " --verify=md5 "
        command += " --serialize_overlap=1 "
    else:
        command += " --verify=0 "

    command += " --time_based=" + str(time_based)+" ";
    command += " --ramp_time=" + str(ramp_time)+" ";
    command += " --numjobs=" + str(numjobs)+" ";
    command += " --size=40mb ";

    if profile_mode == True:
        string = "size_" + str(io_size_bytes) + "_bs_" + str(block_size) + "_qd_" + str(qd) + "_mix_" + str(rw_mix) + "_workload_" + str(workload) + "_run_" + str(run_num)
        command += " --output=" + string + " --output-format=json "

    command += extra_fio_options

    for i in range(0, file_num):
        command += " --name=test" + str(i) + " --filename='trtype=" + str(trtype) + " adrfam=IPv4 traddr=" + str(traddr) + " trsvcid=1158 subnqn=nqn.2019-04.pos\:subsystem" + str(i+1) + " ns=1'"

    if verbose == True:
        print(command)

    ret = subprocess.call(command, shell=True)
    sys.stdout.write(red)
    if ret != 0:
        print("Test {} Failed".format(run_num))
        sys.stdout.write(reset)
        sys.exit(1);
    else:
        print("[TEST {}] ".format(run_num), end='')
        sys.stdout.write(green)
        print("Success. size={} block_size={} qd={} io_pattern={} mix={} cpus_allowed={} runtime={}\n".format(io_size_bytes, block_size, qd, workload, rw_mix, cpus_allowed, run_time_sec))
        sys.stdout.write(reset)
    return


def get_nvme_devices_count():
    output = check_output('lspci | grep -i Non | wc -l', shell=True)
    return int(output)


def get_nvme_devices_bdf():
    output = check_output('lspci | grep -i Non | awk \'{print $1}\'', shell=True).decode("utf-8")
    output = output.split()
    return output


# parse option
parser = optparse.OptionParser()
parser.add_option("-t", "--transport", dest="transport", help="nvmf transport name", default="tcp")
parser.add_option("-i", "--ip", dest="ip", help="nvmf listen ip", default="10.100.11.10")
parser.add_option("-p", "--port", dest="port", type='int', help="nvmf listen port", default=1158)
parser.add_option("-n", "--number", dest="number", type='int', help="number of volumes", default=1)
parser.add_option("-s", "--subnqn",dest="subnqn", help="nvmf subsystem nqn", default="nqn.2019-04.pos\:subsystem")
(options, args) = parser.parse_args()
file_num=options.number
traddr=options.ip

# set up for output file
host_name = os.uname()[1]
result_file_name = host_name + "_perf_output.csv"
columns = "io_size,block_size,q_depth,workload_mix,readwrite,cpus_allowed,run_time,run,read_iops,read_bw(kib/s), \
        read_avg_lat(us),read_min_lat(us),read_max_lat(us),write_iops,write_bw(kib/s),write_avg_lat(us), \
        write_min_lat(us),write_max_lat(us)"
with open(result_file_name, "w+") as result_file:
    result_file.write(columns + "\n")

for i, (s, b, q, m, w, c, t) in enumerate(itertools.product(io_size_bytes, block_size, q_depth, mix, readwrite, cpus_allowed, run_time)):
    run_fio(s, b, q, m, c, i, w, t)

result_file.close()
