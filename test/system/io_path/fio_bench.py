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
import argparse

#######################################################################################
# edit test parameters into these lists to run different workloads
ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"

#select fio plugin for nvmf/e or bdev
fio_plugin='nvme'
ioengine = ibof_root + "lib/spdk/examples/" + str(fio_plugin) + "/fio_plugin/fio_plugin"
#ioengine = ibof_root + "/bin/ibof_bdev_fio_plugin"
spdk_conf = ibof_root + "test/system/nvmf/initiator/spdk_tcp_fio/BDEV.conf"

file_base = 0
file_num = 1
traddr='127.0.0.1'
# could be either nvme or nvmf or bdev
trtype="tcp"
port_num = "1158"
filename = "trtype=tcp adrfam=IPv4 traddr=" + str(traddr) + " trsvcid=" + str(port_num) + " subnqn=nqn.2019-04.pos\:subsystem1 ns=1:"

# the configuration below runs QD 1 & 128. 
# To add set q_depth=['1', '32', '128']
q_depth=['128']

# io_size and block_size specifies the size in bytes of the IO workload.
# To add 64K IOs set io_size = ['4096', '65536'] and block_size = [ '512', '1024', '4096' ]
io_size=['100m']
block_size=['4096', '512b-128k']

# type of I/O pattern : write, read, trim, randread, randwrite, randtrim, readwrite, randrw, rw
readwrite=['write', 'randwrite','randrw']

# verify = True | False | Pattern. applied on sorts of write I/O patterns
verify="True"

# verify_pattern is used when 'verify' option is set to 'pattern'
# Can be set as hexadecimal value with starting '0x' and string with wrapped by ""
# Also filename with wrapped by ''
verify_pattern=""

# buffer_pattern can be used for write buffer fill with user defined pattern.
# please use this if the 'verify' option is not 'pattern'
buffer_pattern=""

# run_time parameter specifies how long to run each test.
# Set run_time = ['10', '600'] to run the test for given seconds
run_time=['2000']

# mixed rw ratio
mix=['100']

# cpuset and affinity to run fio
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
ramp_time='0'
offset=0

# extra fio options
extra_fio_options=" --thread=1 --group_reporting=1 --direct=1"

json_output_file = ""

#######################################################################################

red = "\033[1;31m"
green = "\033[0;32m"
reset = "\033[0;0m"
exit_with_signal = 128

def run_fio(io_size_bytes, block_size, qd, rw_mix, cpus_allowed, run_num, workload, run_time_sec):
    skip_exit_with_signal = False
    sys.stdout.write(red)
    print("[TEST {}] ".format(run_num), end='')
    sys.stdout.write(green)
    print("Started. size={} block_size={} qd={} io_pattern={} mix={} cpus_allowed={} time={}".format(io_size_bytes, block_size, qd, workload, rw_mix, cpus_allowed, run_time_sec))
    sys.stdout.write(reset)
    print("")

    # call fio

    command = "fio "  \
            + " --ioengine=" + str(ioengine) + "" \
            + " --runtime=" + str(run_time_sec) + "" \
            + " --io_size=" + str(io_size_bytes) + "" \
            + " --bs=" + str(block_size) + "" \
            + " --iodepth=" + str(qd) + "" \
            + " --readwrite=" + str(workload) + "" \
            + " --offset=" + str(offset) + ""\

    if fio_plugin == "bdev":
        command += " --spdk_conf=" + str(spdk_conf) + ""

    if cpus_allowed != "":
        command += " --cpus_allowed=" + str(cpus_allowed) + ""

    if workload.find('rw') == -1:
        command += " --norandommap=1 "

    if verify == "True":
        command += " --verify=md5 "
        command += " --serialize_overlap=1 "
    elif verify == "Pattern":
        command += " --verify=pattern"
        command += " --verify_pattern=" + str(verify_pattern)
    else:
        command += " --verify=0 "

    if buffer_pattern != "":
        command += "--buffer_pattern=" + str(buffer_pattern)

    if buffer_pattern != '0':
        command += "--offset=" + str(offset)


    command += " --time_based=" + str(time_based)+" ";
    command += " --ramp_time=" + str(ramp_time)+" ";
    command += " --numjobs=" + str(numjobs)+" ";

    if profile_mode == True:
        if (json_output_file == ""):
            string = "size_" + str(io_size_bytes) + "_bs_" + str(block_size) + "_qd_" + str(qd) + "_mix_" + str(rw_mix) + "_workload_" + str(workload) + "_run_" + str(run_num)
        else:
            string = json_output_file
        command += " --output=" + string + " --output-format=json "

    command += extra_fio_options

    if file_num > 1:
        for i in range(file_base, file_base + file_num):
            if fio_plugin == "nvme":
                command += " --name=test" + str(i) + " --filename='trtype=" + str(trtype) + " adrfam=IPv4 traddr=" + str(traddr) + " trsvcid=" + str(port_num) + " subnqn=nqn.2019-04.pos\:subsystem" + str(i + 1) + " ns=1'"
            else:
                command += " --name=test" + str(i) + "" + " --filename='Nvme" + str(i + 1) + "n1'"
    else:
        command += " --name=test0 --filename='" + filename + "'" 

    if verbose == True:
        print(command)

    ret = subprocess.call(command, shell=True)
    sys.stdout.write(red)
    if ("ibof_bdev_fio_plugin" in ioengine):
        skip_exit_with_signal = True
    if ret != 0 and (skip_exit_with_signal == False or (ret & exit_with_signal) == 0):
        print("Test {} Failed".format(run_num))
        sys.stdout.write(reset)
        sys.exit(1)
    else:
        print("[TEST {}] ".format(run_num), end='')
        sys.stdout.write(green)
        print("Success. size={} block_size={} qd={} io_pattern={} mix={} cpus_allowed={} runtime={}\n".format(io_size_bytes, block_size, qd, workload, rw_mix, cpus_allowed, run_time_sec))
        sys.stdout.write(reset)
    return


def parse_results(io_size_bytes, block_size, qd, rw_mix, cpus_allowed, run_num, workload, run_time_sec):
    results_array = []

    # if json file has results for multiple fio jobs pick the results from the right job
    job_pos = 0

    # generate the next result line that will be added to the output csv file
    results = str(io_size_bytes) + "," + str(block_size) + "," + str(qd) + "," + str(rw_mix) + "," \
            + str(workload) + "," + str(cpus_allowed) + "," + str(run_time_sec) + "," + str(run_num)

    # read the results of this run from the test result file
    string = "size_" + str(io_size_bytes) + "_bs_" + str(block_size) + "_qd_" + str(qd) + "_mix_" + str(rw_mix) + "_workload_" + str(workload) + "_run_" + str(run_num)
    with open(string) as json_file:
        data = json.load(json_file)
        job_name = data['jobs'][job_pos]['jobname']
        # print "fio job name: ", job_name
        if 'lat_ns' in data['jobs'][job_pos]['read']:
            lat = 'lat_ns'
            lat_units = 'ns'
        else:
            lat = 'lat'
            lat_units = 'us'
        read_iops = float(data['jobs'][job_pos]['read']['iops'])
        read_bw = float(data['jobs'][job_pos]['read']['bw'])
        read_avg_lat = float(data['jobs'][job_pos]['read'][lat]['mean'])
        read_min_lat = float(data['jobs'][job_pos]['read'][lat]['min'])
        read_max_lat = float(data['jobs'][job_pos]['read'][lat]['max'])
        write_iops = float(data['jobs'][job_pos]['write']['iops'])
        write_bw = float(data['jobs'][job_pos]['write']['bw'])
        write_avg_lat = float(data['jobs'][job_pos]['write'][lat]['mean'])
        write_min_lat = float(data['jobs'][job_pos]['write'][lat]['min'])
        write_max_lat = float(data['jobs'][job_pos]['write'][lat]['max'])
        print("%-10s" % "io size", "%-10s" % "block size", "%-10s" % "qd", "%-10s" % "mix",
                "%-10s" % "workload type", "%-10s" % "cpu mask",
                "%-10s" % "run time", "%-10s" % "run num",
                "%-15s" % "read iops",
                "%-10s" % "read mbps", "%-15s" % "read avg. lat(" + lat_units + ")",
                "%-15s" % "read min. lat(" + lat_units + ")", "%-15s" % "read max. lat(" + lat_units + ")",
                "%-15s" % "write iops",
                "%-10s" % "write mbps", "%-15s" % "write avg. lat(" + lat_units + ")",
                "%-15s" % "write min. lat(" + lat_units + ")", "%-15s" % "write max. lat(" + lat_units + ")")
        print("%-10s" % io_size_bytes, "%-10s" % block_size, "%-10s" % qd, "%-10s" % rw_mix,
                "%-10s" % workload, "%-10s" % cpus_allowed, "%-10s" % run_time_sec,
                "%-10s" % run_num, "%-15s" % read_iops, "%-10s" % read_bw,
                "%-15s" % read_avg_lat, "%-15s" % read_min_lat, "%-15s" % read_max_lat,
                "%-15s" % write_iops, "%-10s" % write_bw, "%-15s" % write_avg_lat,
                "%-15s" % write_min_lat, "%-15s" % write_max_lat)
        results = results + "," + str(read_iops) + "," + str(read_bw) + "," \
                + str(read_avg_lat) + "," + str(read_min_lat) + "," + str(read_max_lat) \
                + "," + str(write_iops) + "," + str(write_bw) + "," + str(write_avg_lat) \
                + "," + str(write_min_lat) + "," + str(write_max_lat)
        with open(result_file_name, "a") as result_file:
            result_file.write(results + "\n")
        results_array = []
    return


def get_nvme_devices_count():
    output = check_output('lspci | grep -i Non | wc -l', shell=True)
    return int(output)


def get_nvme_devices_bdf():
    output = check_output('lspci | grep -i Non | awk \'{print $1}\'', shell=True).decode("utf-8")
    output = output.split()
    return output


def add_filename_to_conf(conf_file_name, bdf):
    filestring = "filename=trtype=PCIe traddr=0000." + bdf.replace(":", ".") + " ns=1"
    with open(conf_file_name, "a") as conf_file:
        conf_file.write(filestring + "\n")
  
def parse_argument():
    global q_depth
    global io_size
    global ioengine
    global block_size
    global readwrite
    global verify
    global verify_pattern
    global buffer_pattern
    global iter_num
    global ramp_time
    global run_time
    global file_num
    global time_based
    global cpus_allowed
    global numjobs
    global mix
    global filename
    global trtype
    global traddr
    global offset
    global file_base
    global json_output_file
    global profile_mode
    global port_num

    parser = argparse.ArgumentParser(description='Please add fio option to fio_full_bench.py')
    parser.add_argument('--iodepth', required=False, help='Set I/O Queue Depth. Please input without space  Ex) --iodepth="1,32,128"')
    parser.add_argument('--io_size', required=False, help='Set io_size. Please input without space. Please set single number')
    parser.add_argument('--ioengine', required=False, help='Set ioengine Ex) --ioengine=aio')
    parser.add_argument('--bs', required=False, help='Block Size setting. Ex) --bs="4096B,128k"')
    parser.add_argument('--readwrite', required=False, help='Set write read ... Ex) --readwrite="read,write,randread,randwrite"')
    parser.add_argument('--verify', required=False, help='Please verify option as true or false or pattern Ex) --verify=true or --verify=false or --verify=pattern');
    parser.add_argument('--verify_pattern', required=False, help='Please give a verify_pattern for fio Ex) --verify_pattern=\'filename\' or --verify_pattern=0xffffffff or --verify_pattern=\"str\"');
    parser.add_argument('--buffer_pattern', required=False, help='Please give a buffer_pattern for fio Ex) --buffer_pattern=\'filename\' or --buffer_pattern=0xffffffff or --buffer_pattern=\"str\"');
    parser.add_argument('--ramp_time', required=False, help='Set ramp time. ')
    parser.add_argument('--run_time', required=False, help='Set run time. please set as --time_based=1 also ')
    parser.add_argument('--file_num', required=False, help='Set number of files for tests ')
    parser.add_argument('--file_base', required=False, help='Set base number of file test')
    parser.add_argument('--time_based', required=False, help='Set time Based option')
    parser.add_argument('--cpus_allowed', required=False, help='Set cpu list, --cpus_allowed=2-31,41')
    parser.add_argument('--numjobs', required=False, help='Set numberof jobs ')
    parser.add_argument('--mix', required=False, help='Set mix percentage ')
    parser.add_argument('--target_volume', required=False, help='Set target volume to io ')
    parser.add_argument('--offset', required=False, help='Set start offset ')
    parser.add_argument('--filename', required=False, help='Set FileName')
    parser.add_argument('-i', '--traddr', required=False, help='Set target ip address ')
    parser.add_argument('-t', '--trtype', required=False, help='Set transfer type (tcp or rdma) ')
    parser.add_argument('-p', '--port', required=False, help='Set NVMeoF listening port ')
    parser.add_argument('-n', '--nsid', required=False, help='Set Namespace id ')
    parser.add_argument('-j', '--json_output_file', required=False, help='Output will be json file')

    args = parser.parse_args()
    if(args.iodepth is not None):
        q_depth=args.iodepth.split(',')
    if(args.io_size is not None):
        io_size=args.io_size.split(',')
    if(args.ioengine is not None):
        ioengine=args.ioengine
    if(args.bs is not None):
        block_size=args.bs.split(',')
    if(args.readwrite is not None):
        readwrite=args.readwrite.split(',')
    if(args.verify is not None):
        if(args.verify == "true" or args.verify == "True" or args.verify == "1"):
            verify = "True"
        elif(args.verify == "pattern" or args.verify == "Pattern"):
            verify = "Pattern"
            verify_pattern = args.verify_pattern
        else:
            verify = False
    if(args.buffer_pattern is not None):
        buffer_pattern = args.buffer_pattern
    if(args.ramp_time is not None):
        ramp_time=args.ramp_time
    if(args.run_time is not None):
        run_time=args.run_time.split(',')
    if(args.file_num is not None):
        file_num=int(args.file_num,0)
    if(args.file_base is not None):
        file_base = int(args.file_base, 0)
    if(args.time_based is not None):
        time_based=args.time_based
    if(args.cpus_allowed is not None):
        cpus_allowed=[args.cpus_allowed]
    if(args.numjobs is not None):
        numjobs=args.numjobs
    if(args.traddr is not None):
        filename=filename.replace("traddr=10.100.11.1","traddr="+args.traddr)
        traddr=args.traddr #for multi subsystem test
    if(args.trtype is not None):
        filename=filename.replace("trtype=tcp","trtype="+args.trtype)
        trtype=args.trtype #for multi subsystem test
    if(args.target_volume):
        if(1 == file_num):
            filename=filename.replace("subsystem1", "subsystem"+args.target_volume)
    if(args.offset is not None):
        offset=args.offset
    if(args.filename):
        filename = args.filename
    if(args.json_output_file is not None):
        json_output_file = args.json_output_file
        profile_mode = True
    if(args.port is not None):
        port_num = args.port

# set up for output file
host_name = os.uname()[1]
result_file_name = host_name + "_perf_output.csv"
columns = "io_size,block_size,q_depth,workload_mix,readwrite,cpus_allowed,run_time,run,read_iops,read_bw(kib/s), \
        read_avg_lat(us),read_min_lat(us),read_max_lat(us),write_iops,write_bw(kib/s),write_avg_lat(us), \
        write_min_lat(us),write_max_lat(us)"

parse_argument()

with open(result_file_name, "w+") as result_file:
    result_file.write(columns + "\n")

for i, (s, b, q, m, w, c, t) in enumerate(itertools.product(io_size, block_size, q_depth, mix, readwrite, cpus_allowed, run_time)):
    run_fio(s, b, q, m, c, i, w, t)
    if profile_mode == True:
        parse_results(s, b, q, m, c, i, w, t)

result_file.close()
