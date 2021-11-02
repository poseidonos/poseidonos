#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil
import sys
import paramiko
import time
import threading
import remote_control

default_fabric_ip = ["10.100.2.16", "10.100.3.16"]
default_port_num = ["1158", "1159"]
default_initiator_ip = ["10.1.2.30", "10.1.2.31"]
default_target_ip = "10.1.2.16"
default_initiator_id = "root"
default_initiator_pw = "psd"
default_bringup = True

fabric_ip = []
initiator_ip = []
port_num = []

default_ibofos_root = "/home/ibof/ibofos"
config_dir = "/etc/pos/"

ibofos_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"


def bring_up_ibofos_target():
    global args, fabric_ip
    start_ibofos_script = ibofos_root + "test/regression/start_poseidonos.sh"
#   start_ibofos_script = ibofos_root + "/bin/poseidonos --wait-for-rpc &"
#   os.system(start_ibofos_script)
    subprocess.call(start_ibofos_script)
#   time.sleep(30)
#   os.system(ibofos_root + "/lib/spdk/scripts/rpc.py framework_start_init")
    bring_up_script = ibofos_root + "/test/system/io_path/setup_ibofos_nvmf_volume_multi_node.sh"
    subprocess.call([bring_up_script,
                     "-a", fabric_ip[0],
                     "-A", fabric_ip[1],
                     "-s", "33",
                     "-S", "33",
                     "-B", "21474836480",
                     "-v", "33",
                     "-V", "33",
                     "-b", "8192",
                     "-w", "4096",
                     "-i", "TRUE",
                     "-p", "none"
                     ])


def kill_ibofos():
    for proc in psutil.process_iter():
        try:
            if "poseidonos" in proc.name():
                proc.kill()
                proc.wait()
        except psutil.NoSuchProcess:
            pass


def bring_up_ibofos():
    kill_ibofos()
    print("Try to execute ibofos at target")
    bring_up_ibofos_target()


def get_performance_bandwith_MB(node, ioType):
    global args, initiator_ip
    initiator_script = args.ibofos_root + \
        "/test/system/io_path/fio_output_parser.py -t " + ioType
    result_str = remote_control.remote_execute(
        initiator_ip[node], args.initiator_id, args.initiator_pw, initiator_script, False, False).split()
    result1 = result_str[0]
    result2 = result_str[3]
    result3 = result_str[2]
    bw = int(result1.rstrip("\n"), 0) / 1000
    latency_99_99 = int(result2.rstrip("\n"), 0) / 1000
    latency_mean = float(result3.rstrip("\n")) / 1000
    print("result : %.2f MB/s %.2f %.2f " % (bw, latency_99_99, latency_mean) + "us")

    return (bw, latency_99_99, latency_mean)


def execute_fio_in_initiator(readwrite, block_size, qd, node):
    global args, initiator_ip, fabric_ip, port_num
    print("Execute fio at initiator readwrite=%s block_size=%s qd=%s" %
          (readwrite, block_size, qd))
    ramp_time = "30"
    run_time = "30"
    num_jobs = "1"

    file_num = "33"
    # initiator's result file
    result_file = "/tmp/fio_output.json"
    remote_control.remote_execute(initiator_ip[node], args.initiator_id,
                   args.initiator_pw, "rm -rf " + result_file)
    initiator_script = args.ibofos_root + "/test/system/io_path/fio_bench.py -i " + fabric_ip[node] + " --readwrite=" + readwrite +\
        " --ramp_time=" + ramp_time + " --bs=" + block_size + " --run_time=" + run_time + " --time_based=1 " + " --file_num=" + file_num + " --verify=" + "false" +\
        " --iodepth=" + qd + " --numjobs=" + num_jobs + " --io_size=8g --port=" + port_num[node] + " "
    if (node != 0):
        initiator_script += " --file_base=33"

    initiator_script += " --json_output_file=" + result_file
    print(initiator_script)
    t = remote_control.remote_execute(initiator_ip[node], args.initiator_id,
                       args.initiator_pw, initiator_script, False, True)
    time.sleep(1)
    return t


def parse_argument():
    global fabric_ip, initiator_ip, port_num, args
    parser = argparse.ArgumentParser(description='Filebench Test')
    parser.add_argument('-f1', '--fabric_ip1', default=default_fabric_ip[0],
                        help='Set target fabric IP, default: ' + default_fabric_ip[0])
    parser.add_argument('-f2', '--fabric_ip2', default=default_fabric_ip[1],
                        help='Set target fabric IP, default: ' + default_fabric_ip[0])
    parser.add_argument('-i1', '--initiator_ip1', default=default_initiator_ip[0],
                        help='Set initiator IP, default: ' + default_initiator_ip[0])
    parser.add_argument('-i2', '--initiator_ip2', default=default_initiator_ip[1],
                        help='Set initiator IP, default: ' + default_initiator_ip[0])

    parser.add_argument('-p1', '--port_num1', default=default_port_num[0],
                        help='Set port IP, default: ' + default_port_num[0])
    parser.add_argument('-p2', '--port_num2', default=default_port_num[1],
                        help='Set port IP, default: ' + default_port_num[0])

    parser.add_argument('--initiator_pw', default=default_initiator_pw,
                        help='Set initiator PW, default: ' + default_initiator_pw)
    parser.add_argument('--initiator_id', default=default_initiator_id,
                        help='Set initiator ID, default: ' + default_initiator_id)
    parser.add_argument('-r', '--ibofos_root', default=default_ibofos_root,
                        help='Set ibofos root path, default: ' + default_ibofos_root)
    parser.add_argument('-a', '--auto_bringup', default=default_bringup,
                        help='Automatically execute bringup script')
    args = parser.parse_args()
    fabric_ip.append(args.fabric_ip1)
    fabric_ip.append(args.fabric_ip2)

    port_num.append(args.port_num1)
    port_num.append(args.port_num2)
    print(port_num[0], port_num[1])

    initiator_ip.append(args.initiator_ip1)
    initiator_ip.append(args.initiator_ip2)
    print(args)


def terminate_ibofos():
    unmount_ibofos_command = ibofos_root + "/bin/poseidonos-cli array unmount --array-name POSArray1 --force"
    os.system(unmount_ibofos_command)
    unmount_ibofos_command = ibofos_root + "/bin/poseidonos-cli array unmount --array-name POSArray2 --force"
    os.system(unmount_ibofos_command)
    stop_pos_command = ibofos_root + "/bin/poseidonos-cli system stop --force"
    os.system(stop_pos_command)
    check_ibofos_command = "pgrep -c poseidonos"
    result = os.system(check_ibofos_command)
    print("Wait exit")
    while (int(result) == 0):
        result = os.system(check_ibofos_command)
        time.sleep(0.5)


def test_scenario():
    os.sched_setaffinity(0, [os.cpu_count() - 1])
    os.system("rm -rf /dev/shm/ibof_nvmf_trace.pid*")
    os.system("mkdir " + config_dir)
    if (os.path.isfile(config_dir + "pos.conf")):
        os.system("cp " + config_dir + "pos.conf " +
                  config_dir + "pos.conf.reserved -rf")
    os.system("cp " + ibofos_root + "config/pos_multi_array_perf.conf " +
              config_dir + "pos.conf -rf")

    if (args.auto_bringup == True):
        bring_up_ibofos()

    readwrite = ["write", "read", "randwrite", "randread", "randwrite", "randread", "randwrite", "randread"]
    io_depth = ["4", "4", "128", "128", "1", "1", "32", "32"]
    block_size = ["128k", "128k", "4k", "4k", "4k", "4k", "4k", "4k"]
    dict_real = {}
    dict_real_latency = {}
    for index in range(0, 8):
        t1 = execute_fio_in_initiator(readwrite[index], block_size[index], io_depth[index], 0)
        t2 = execute_fio_in_initiator(readwrite[index], block_size[index], io_depth[index], 1)
        t1.join()
        t2.join()
        (bw1, latency_99_99_1, latency_1) = get_performance_bandwith_MB(0, readwrite[index])
        (bw2, latency_99_99_2, latency_2) = get_performance_bandwith_MB(1, readwrite[index])

        latency_99_99 = (latency_99_99_1 + latency_99_99_2) / 2
        latency = (latency_1 + latency_2) / 2
        bw = (bw1 + bw2)
        dict_real[readwrite[index] + '_' + io_depth[index] + '_' + block_size[index]] = (bw, latency, latency_99_99)

    print("############ Result ##############")
    for key in dict_real:
        print(key, "BW (MB/s), avg latency (us), 99.99 (us) = ", dict_real[key])


if __name__ == "__main__":
    parse_argument()
    try:
        test_scenario()
        terminate_ibofos()

    except subprocess.CalledProcessError:
        print("Fail to execute command")
        terminate_ibofos()
        sys.exit(-1)

    sys.exit(0)
