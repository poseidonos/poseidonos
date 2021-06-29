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
ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../.."
ioengine = ibof_root + "/lib/spdk/examples/nvme/fio_plugin/fio_plugin"

# 1 namespace
#filename='trtype=pcie traddr=0000.02.00.0 ns=1'

# the configuration below runs QD 1 & 128.
# To add set q_depth=['1', '32', '128']
q_depth=['128']

# io_size and block_size specifies the size in bytes of the IO workload.
# To add 64K IOs set io_size = ['4096', '65536'] and block_size = [ '512', '1024', '4096' ]
io_size=['10M']
block_size=['4K']

# type of I/O pattern : write, read, trim, randread, randwrite, randtrim, readwrite, randrw
readwrite=['write']

# verify = True | False. applied on sorts of write I/O patterns
verify=False

# run_time parameter specifies how long to run each test.
# Set run_time = ['10', '600'] to run the test for given seconds
run_time=['60']

# mixed rw ratio
mix=['100']

# cpu to run fio
core_mask=['0x1']

# iter_num parameter is used to run the test multiple times.
# set iter_num = ['1', '2', '3'] to repeat each test 3 times
iter_num=['1']

# setting profile_mode True | False. True will remains profile json file and .csv result file
profile_mode=False

# verbose = True | False. setting True will show more fio log
verbose=True

# extra fio options
extra_fio_options=" --numjobs=1 --ramp_time=0 --norandommap=1 --bs_unaligned=1 "

#######################################################################################

red = "\033[1;31m"
green = "\033[0;32m"
reset = "\033[0;0m"

def run_fio(filename, io_size_bytes, block_size, qd, rw_mix, cpu_mask, run_num, workload, run_time_sec, io_offset, pattern, verify_data):
    sys.stdout.write(red)
    print("[TEST {}] ".format(run_num), end='')
    sys.stdout.write(green)
    print("Started. size={} block_size={} qd={} io_pattern={} mix={} cpu mask={} time={}".format(io_size_bytes, block_size, qd, workload, rw_mix, cpu_mask, run_time_sec))
    sys.stdout.write(reset)
    print("")

    # call fio

    command = "fio --thread=1 --group_reporting=1 --direct=1 " \
            + " --time_based=" + str(0) + "" \
            + " --ioengine=" + str(ioengine) + "" \
            + " --runtime=" + str(run_time_sec) + "" \
            + " --size=" + str(io_size_bytes) + "" \
            + " --bs=" + str(block_size) + "" \
            + " --iodepth=" + str(qd) + "" \
            + " --readwrite=" + str(workload)

    if verify == True:
        command += " --verify=md5 "
    else:
        command += " --verify=0 "

    if profile_mode == True:
        string = "size_" + str(io_size_bytes) + "_bs_" + str(block_size) + "_qd_" + str(qd) + "_mix_" + str(rw_mix) + "_workload_" + str(workload) + "_run_" + str(run_num)
        command += " --output=" + string + " --output-format=json "
    elif verify_data != 0:
        if workload == "read":
            command += " --verify_fatal=1 --do_verify=1 --output=verify" + str(verify_data) + ".json --output-format=json "

    command += " --offset=" + io_offset
    if workload == "write":
        command += " --buffer_pattern=" + pattern
    elif workload == "read":
        command += " --verify=pattern --verify_pattern=" + pattern
    
    command += extra_fio_options
    command += " --name=test --filename='" + filename + "'"

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
        print("Success. size={} block_size={} qd={} io_pattern={} mix={} cpu_mask={} runtime={}\n".format(io_size_bytes, block_size, qd, workload, rw_mix, cpu_mask, run_time_sec))
        sys.stdout.write(reset)
    return


def parse_results(io_size_bytes, block_size, qd, rw_mix, cpu_mask, run_num, workload, run_time_sec):
    results_array = []

    # if json file has results for multiple fio jobs pick the results from the right job
    job_pos = 0

    # generate the next result line that will be added to the output csv file
    results = str(io_size_bytes) + "," + str(block_size) + "," + str(qd) + "," + str(rw_mix) + "," \
            + str(workload) + "," + str(cpu_mask) + "," + str(run_time_sec) + "," + str(run_num)

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
                "%-10s" % workload, "%-10s" % cpu_mask, "%-10s" % run_time_sec,
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


# parse option
parser = optparse.OptionParser()
parser.add_option("-s", "--iosize", dest="iosize", help="io size", default="10M")
parser.add_option("-t", "--transport", dest="transport", help="nvmf transport name", default="rdma")
parser.add_option("-i", "--ip", dest="ip", help="nvmf listen ip", default="10.100.11.23")
parser.add_option("-p", "--port", dest="port", type='int', help="nvmf listen port", default=1158)
parser.add_option("-n", "--nvol", dest="nvol", type='int', help="volume id", default=1)
parser.add_option("-o", "--offset", dest="offset", help="io start offset", default="0")
parser.add_option("-v", "--verifypattern", dest="verifypattern", help="write&verify pattern", default='\"ABCD\"')
parser.add_option("-w", "--workload", dest="readwrite", help="read/write workload", default=readwrite)
parser.add_option("-d", "--verifydata", dest="verifydata", type='int', help="output data for verify", default=0)

(options, args) = parser.parse_args()
filename="trtype=" + options.transport + " adrfam=IPv4" + " traddr=" + options.ip + " trsvcid=" + str(options.port) + " subnqn=nqn.2019-04.pos\:subsystem" + str(options.nvol) + " ns=1";
print(filename);
if type(options.readwrite) == str:
    readwrite=[options.readwrite]
verify_data=options.nvol

# set up for output file
host_name = os.uname()[1]
result_file_name = host_name + "_perf_output.csv"
columns = "io_size,block_size,q_depth,workload_mix,readwrite,core_mask,run_time,run,read_iops,read_bw(kib/s), \
        read_avg_lat(us),read_min_lat(us),read_max_lat(us),write_iops,write_bw(kib/s),write_avg_lat(us), \
        write_min_lat(us),write_max_lat(us)"
with open(result_file_name, "w+") as result_file:
    result_file.write(columns + "\n")

for i, (b, q, m, w, c, t) in enumerate(itertools.product(block_size, q_depth, mix, readwrite, core_mask, run_time)):
    run_fio(filename, options.iosize, b, q, m, c, i, w, t, options.offset, options.verifypattern, verify_data)
    if profile_mode == True:
        parse_results(s, b, q, m, c, i, w, t)

result_file.close()
