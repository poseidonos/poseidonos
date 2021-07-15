#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil
import sys
import paramiko
import time
import threading

default_fabric_ip = ["10.100.2.16", "10.100.3.16"]
default_initiator_ip = ["10.1.2.30", "10.1.2.31"]
default_target_ip = "10.1.2.16"
default_initiator_id = "root"
default_initiator_pw = "psd"
default_bringup = True

fabric_ip = []
initiator_ip = []

default_ibofos_root = "/home/ibof/ibofos"
config_dir = "/etc/pos/"


def _remote_execute(ip, id, pw, command, stderr_report=False):
    cli = paramiko.SSHClient()
    cli.load_system_host_keys()
    cli.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    cli.connect(ip, port=22, username=id, password=pw)
    stdin, stdout, stderr = cli.exec_command(command)
    result = ""
    if (stderr_report is True):
        for line in iter(stderr.readline, ""):
            print(line, end="")
            result += line
    for line in iter(stdout.readline, ""):
        print(line, end="")
        result += line
    cli.close()
    return result


class Worker(threading.Thread):
    def __init__(self, name):
        super().__init__()
        self.name = name

    def run(self):
        global thread_param_ip, thread_param_id, thread_param_pw, thread_param_command
        print("sub thread start ", threading.currentThread().getName())
        remote_execute(thread_param_ip, thread_param_id, thread_param_pw, thread_param_command)
        print("sub thread end ", threading.currentThread().getName())


def remote_execute(ip, id, pw, command, stderr_report=False, asyncflag=False):
    if (asyncflag is False):
        return _remote_execute(ip, id, pw, command, stderr_report)
    else:
        global thread_param_ip, thread_param_id, thread_param_pw, thread_param_command
        thread_param_ip = ip
        thread_param_id = id
        thread_param_pw = pw
        thread_param_command = command
        t = Worker("async thread")
        t.start()
        return t


ibofos_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"


def bring_up_ibofos_target():
    global args, fabric_ip
    start_ibofos_script = ibofos_root + "test/regression/start_poseidonos.sh"
    subprocess.call(start_ibofos_script)
    bring_up_script = ibofos_root + "/test/system/io_path/setup_ibofos_nvmf_volume_multi_node.sh"
    subprocess.call([bring_up_script,
                     "-a", fabric_ip[0],
                     "-A", fabric_ip[1],
                     "-s", "33",
                     "-S", "32",
                     "-B", "21474836480",
                     "-v", "33",
                     "-V", "32",
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
    result = remote_execute(
        initiator_ip[node], args.initiator_id, args.initiator_pw, initiator_script, False, False).split()[0]
    bw = int(result.rstrip("\n"), 0) / 1000
    print("result : %s " % (bw) + "MB/s")

    return bw


def execute_fio_in_initiator(readwrite, block_size, qd, node):
    global args, initiator_ip, fabric_ip
    print("Execute fio at initiator readwrite=%s block_size=%s qd=%s" %
          (readwrite, block_size, qd))
    ramp_time = "15"
    run_time = "5"
    num_jobs = "1"

    if (node == 0):
        file_num = "33"
    else:
        file_num = "32"
    # initiator's result file
    result_file = "/tmp/fio_output.json"
    remote_execute(initiator_ip[node], args.initiator_id,
                   args.initiator_pw, "rm -rf " + result_file)
    initiator_script = args.ibofos_root + "/test/system/io_path/fio_bench.py -i " + fabric_ip[node] + " --readwrite=" + readwrite +\
        " --ramp_time=" + ramp_time + " --bs=" + block_size + " --run_time=" + run_time + " --time_based=1 " + " --file_num=" + file_num + " --verify=" + "false" +\
        " --iodepth=" + qd + " --numjobs=" + num_jobs + " --io_size=8g "
    if (node != 0):
        initiator_script += " --file_base=33"

    initiator_script += " --json_output_file=" + result_file
    print(initiator_script)
    t = remote_execute(initiator_ip[node], args.initiator_id,
                       args.initiator_pw, initiator_script, False, True)
    time.sleep(1)
    return t


def parse_argument():
    global fabric_ip, initiator_ip, args
    parser = argparse.ArgumentParser(description='Filebench Test')
    parser.add_argument('-f1', '--fabric_ip1', default=default_fabric_ip[0],
                        help='Set target fabric IP, default: ' + default_fabric_ip[0])
    parser.add_argument('-f2', '--fabric_ip2', default=default_fabric_ip[1],
                        help='Set target fabric IP, default: ' + default_fabric_ip[0])
    parser.add_argument('-i1', '--initiator_ip1', default=default_initiator_ip[0],
                        help='Set initiator IP, default: ' + default_initiator_ip[0])
    parser.add_argument('-i2', '--initiator_ip2', default=default_initiator_ip[1],
                        help='Set initiator IP, default: ' + default_initiator_ip[0])

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
    initiator_ip.append(args.initiator_ip1)
    initiator_ip.append(args.initiator_ip2)
    print(args)


def terminate_ibofos():
    unmount_ibofos_command = ibofos_root + "/bin/cli array unmount --name POSArray1"
    os.system(unmount_ibofos_command)
    unmount_ibofos_command = ibofos_root + "/bin/cli array unmount --name POSArray2"
    os.system(unmount_ibofos_command)
    exit_ibofos_command = ibofos_root + "/bin/cli system exit"
    os.system(exit_ibofos_command)
    check_ibofos_command = "pgrep -c ibofos"
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

    readwrite = ["write", "read", "randwrite", "randread"]
    io_depth = ["4", "4", "128", "128"]
    block_size = ["128k", "128k", "4k", "4k"]
    dict_real = {}
    for index in range(0, 4):
        t1 = execute_fio_in_initiator(readwrite[index], block_size[index], io_depth[index], 0)
        t2 = execute_fio_in_initiator(readwrite[index], block_size[index], io_depth[index], 1)
        t1.join()
        t2.join()
        bw1 = get_performance_bandwith_MB(0, readwrite[index])
        bw2 = get_performance_bandwith_MB(1, readwrite[index])
        bw = bw1 + bw2
        dict_real[readwrite[index]] = bw

    print("############ Result ##############")
    for key in dict_real:
        print(key, "BW = ", dict_real[key], "MiB/s")


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
