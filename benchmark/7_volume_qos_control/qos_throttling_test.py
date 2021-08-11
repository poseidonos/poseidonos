#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil
import sys
import paramiko
import time

cli = "bin/cli qos vol_policy"
default_ibofos_root = "/home/ibof/ibofos"
config_dir = "/etc/pos/"


ibofos_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"


def get_performance_bandwith_MiB():
    global args
    initiator_script = args.ibofos_root + \
        "/test/system/qos/initiator_parse_fio_result.py"
    result = remote_execute(
        args.initiator_ip, args.initiator_id, args.initiator_pw, initiator_script)
    print("result : %s " % result.rstrip("\n") + "MiB/s")
    return result.rstrip("\n")



def parse_argument():
    parser = argparse.ArgumentParser(description='Filebench Test')
    parser.add_argument('-f', '--fabric_ip', default=default_fabric_ip,
                        help='Set target fabric IP, default: ' + default_fabric_ip)
    parser.add_argument('-i', '--initiator_ip', default=default_initiator_ip,
                        help='Set initiator IP, default: ' + default_initiator_ip)
    parser.add_argument('--initiator_pw', default=default_initiator_pw,
                        help='Set initiator PW, default: ' + default_initiator_pw)
    parser.add_argument('--initiator_id', default=default_initiator_id,
                        help='Set initiator ID, default: ' + default_initiator_id)
    parser.add_argument('-r', '--ibofos_root', default=default_ibofos_root,
                        help='Set ibofos root path, default: ' + default_ibofos_root)
    global args
    args = parser.parse_args()
    print(args)


def terminate_ibofos():
    unmount_ibofos_command = ibofos_root + "/bin/cli request unmount_ibofos"
    os.system(unmount_ibofos_command)
    exit_ibofos_command = ibofos_root + "/bin/cli request exit_ibofos"
    os.system(exit_ibofos_command)
    check_ibofos_command = "pgrep -c ibofos"
    result = os.system(check_ibofos_command)
    print("Wait exit")
    while (int(result) == 0):
        result = os.system(check_ibofos_command)
        time.sleep(0.5)


def cli_max_bw_setting(max_bw, ratio):
    print("cli throttling %s %d percent" % (max_bw, int(ratio * 100)))
    cli_cmd = ibofos_root + cli + \
        " -a POSArray -v vol1 --maxbw " + str(int(max_bw * ratio))
    print(cli_cmd)
    ret = os.system(cli_cmd)
    print("return code : %d" % ret)
    return ret


def cli_max_iops_setting(max_iops, ratio):
    print("cli throttling %s %d percent" % (max_iops, int(ratio * 100)))
    cli_cmd = ibofos_root + cli + \
        " -a POSArray -v vol1 --maxiops " + \
        str(int(max_iops * ratio))
    print(cli_cmd)
    ret = os.system(cli_cmd)
    print("return code : %d" % ret)
    return ret


def test_scenario():
    os.system("rm -rf /dev/shm/ibof_nvmf_trace.pid*")
    os.system("mkdir " + config_dir)

    if (os.path.isfile(config_dir + "pos.conf")):
        os.system("cp " + config_dir + "pos.conf " +
                  config_dir + "pos.conf.reserved -rf")

    os.system("cp " + ibofos_root + "config/ibofos_for_perf_ci.conf " +
              config_dir + "pos.conf -rf")

    bring_up_ibofos()

    ratios = [1, 0.7, 0.5, 0.3, 0.1]
    readwrite = ["write", "read", "randwrite", "randread"]
    io_depth = ["4", "4", "128", "128"]
    block_size = ["128k", "128k", "4k", "4k"]
    dict_real = {}
    dict_expect = {}
    dict_real_iops = {}
    dict_expect_iops = {}
    perf = []
    for index in range(0, 4):
        max_bw = execute_fio_in_initiator(
            readwrite[index], block_size[index], io_depth[index])
        max_iops = int(float(int(max_bw)) /
                       int(block_size[index].rstrip("k"))) * 1024
        for ratio in ratios:
            cli_max_bw_setting(int(0), ratio)
            ret = cli_max_bw_setting(int(max_bw), ratio)
            bw = "0"
            if (int(int(max_bw) * ratio) >= 10 and ret == 0):
                bw = execute_fio_in_initiator(
                    readwrite[index], block_size[index], io_depth[index])
            dict_str = readwrite[index] + " io_depth=" + io_depth[index] + \
                " block_size=" + block_size[index] + " ratio=" + str(ratio)
            dict_expect[dict_str] = int(int(max_bw) * ratio)
            dict_real[dict_str] = int(bw)
            cli_max_bw_setting(int(0), ratio)

            cli_max_iops_setting(int(0), ratio)
            ret = cli_max_iops_setting(int(max_iops), ratio)
            bw = "0"
            if (int(int(max_iops) * ratio) >= 10 and ret == 0):
                bw = execute_fio_in_initiator(
                    readwrite[index], block_size[index], io_depth[index])
            dict_str = readwrite[index] + " io_depth=" + io_depth[index] + \
                " block_size=" + block_size[index] + " ratio=" + str(ratio)
            dict_expect_iops[dict_str] = int(int(max_iops) * ratio)
            dict_real_iops[dict_str] = int(
                float(int(bw)) / int(block_size[index].rstrip("k")))
            cli_max_iops_setting(int(0), ratio)

    print("############ Result ##############")
    for key in dict_real:
        print(key, "BW setting = ", dict_expect[key], "MiB/s", "  real = ", dict_real[key],
              "MiB/s  IOPS setting = ", dict_expect_iops[key], "kIOPs  real = ", dict_real_iops[key], "kIOPs")

