#!/usr/bin/python3

import os
import subprocess
import sys
import psutil
import argparse
import time

current_path = os.path.dirname(os.path.realpath(__file__))
lib_path = os.path.dirname(current_path) + "/../../lib"
sys.path.insert(1, lib_path)
import remote_procedure

default_pos_root = "/home/ibof/ibofos/"
default_pos_config_filename = "ibofos_for_perf_ci.conf"
default_pos_config_path = "/etc/pos/pos.conf"
default_fabric_ip = "127.0.0.1"
default_initiator_ip = "127.0.0.1"
default_target_ip = "127.0.0.1"
default_initiator_id = "root"
default_initiator_pw = "bamboo"
default_target_id = "root"
default_target_pw = "bamboo"
default_user_data_device_list="unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3,unvme-ns-4,unvme-ns-5,unvme-ns-6"
default_subsystem_count="31"
default_volume_count="31"
default_target_nic="enp59s0"
default_net_irq_cpulist="46-55"

default_runtime="15"
default_io_size="2g"
default_qd_128k="4"
default_qd_4k="128"
default_time_based="1"
default_transport="tcp"

default_sw_128k_limit="9500"
default_sr_128k_limit="11000"
default_rw_4k_limit="5000"
default_rr_4k_limit="8300"
default_test_revision="temp"
default_test_name="POS_devel"
#----------------------------------------------------------------------------------------------------------------------
irq_dedication_enable="true"
write_buffer_size_in_mb="4096"
spare_device_list="none"
num_jobs="1"
test_dir="./test/system/nvmf/initiator/automated/"
array_name="POSArray"
spdk_version="spdk-20.10"
#--------------------------------------------------------------------------------------------------------------------
# currently not using in perf ci
default_ioat_qd="44,128"
default_non_ioat_qd="128,128"

def set_performance_config():
    print ("Copy pos_for_perf_ci.conf to pos.conf")
    copy_perf_cofig_script = "sudo cp " + args.pos_root + "config/" + args.config_filename + " " + args.config_path 
    print(copy_perf_cofig_script)
    remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, copy_perf_cofig_script)
    print("Copied pos_for_perf_ci.conf to pos.conf successfully")

def check_request_volume_mounted(result):
    print("check request volumes are mounted")
    output = result.splitlines()
    cnt = 0
    for line in output:
        name = "\"bdev_name\": \"bdev_" + str(cnt)
        if name in line:
            cnt += 1
    if cnt != int(args.volume_count):
        raise Exception(args.target_ip, "check every requested volume is mounted")
    else:
        print("All request volumes are mounted")
    
def bring_up_ibofos():
    print ("start poseidon os")
    start_pos_script = "cd " + args.pos_root +  "; sudo ./test/regression/start_poseidonos.sh"
    print(start_pos_script)
    remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, start_pos_script)
    print("Pos started successfully")
    bring_up_script = "cd " + args.pos_root + "; sudo ./test/system/io_path/setup_ibofos_nvmf_volume.sh -a " + \
            args.fabric_ip + " -i " + irq_dedication_enable + " -w " + write_buffer_size_in_mb + " -u " + \
            args.userdata_device_list +" -p " + spare_device_list + " -s " + \
            args.subsystem_count + " -v " + args.volume_count + " -n " + args.target_nic + " -q " + args.net_irq_cpulist
    print(bring_up_script)
    result = remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, bring_up_script)
    print("Pos bring up success")

    time.sleep(10)
    print("Start telemetry")
    start_telemetry_script = args.pos_root + "/bin/poseidonos-cli telemetry start"
    print(start_telemetry_script)
    remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, start_telemetry_script)
    print("Telemetry is available")
    # check_request_volume_mounted(result)

def execute_performance_test():
    print ("start perf test")
    perf_4k_test_script = "cd " + args.pos_root + "; sudo " + test_dir + "perf.py -r " + args.runtime + \
            " -b 4k -s " + args.io_size + " -n " + num_jobs + " -t " + args.time_based + " -p " + args.transport + \
            " -i " + args.fabric_ip + " -x " + args.qd_4k + " -y " + args.qd_4k
    print(perf_4k_test_script)
    remote_procedure.execute(args.initiator_ip, args.initiator_id, args.initiator_pw, perf_4k_test_script)

    perf_128k_test_script = "cd " + args.pos_root + "; sudo " + test_dir + "perf.py -r " + args.runtime + \
            " -b 128k -s " + args.io_size + " -n " + num_jobs + " -t " + args.time_based + " -p " + args.transport + \
            " -i " + args.fabric_ip + " -x " + args.qd_128k + " -y " + args.qd_128k
    remote_procedure.execute(args.initiator_ip, args.initiator_id, args.initiator_pw, perf_128k_test_script)

    print ("perf test done")

def convert_result_to_csv():
    print("convert performance result to csv file")
    convert_csv_script = "cd " + args.pos_root + "; sudo " + test_dir + "convert_csv.py -r " + args.test_revision +\
            " -sw " + args.sw_limit + " -sr " + args.sr_limit + " -rw " + args.rw_limit + " -rr " + args.rr_limit +\
            " -t " + args.test_name
    print(convert_csv_script)
    remote_procedure.execute(args.initiator_ip, args.initiator_id, args.initiator_pw, convert_csv_script)
    
    print("convert performace result to csv file done")

def terminate_pos():
    exit_pos_command = "pkill -9 pos"
    print(exit_pos_command)
    remote_procedure.execute(args.target_ip, args.target_id, args.target_pw, exit_pos_command)

def parse_argument():
    parser = argparse.ArgumentParser(description='Performance Test')
    parser.add_argument('-i', '--initiator_ip', default=default_initiator_ip,\
            help='Set initiator IP, default: '+ default_initiator_ip)
    parser.add_argument('-r', '--pos_root', default=default_pos_root,\
            help='Set poseidonos root path, default: ' + default_pos_root)
    parser.add_argument('-cf', '--config_filename', default=default_pos_config_filename,\
            help='Set pos config filename, default: ' + default_pos_config_filename)
    parser.add_argument('-cp', '--config_path', default=default_pos_config_path,\
            help='Set pos config path, default: ' +  default_pos_config_path)
    parser.add_argument('-f', '--fabric_ip', default=default_fabric_ip,\
            help='Set target fabric IP, default: ' + default_fabric_ip)
    parser.add_argument('-t', '--target_ip', default=default_target_ip,\
            help='Set target IP, default: ' + default_target_ip)
    parser.add_argument('--target_pw', default=default_target_pw,\
            help='Set target PW, default: ' + default_target_pw)
    parser.add_argument('--initiator_pw', default=default_initiator_pw,\
            help='Set initiator PW, default: ' + default_initiator_pw)
    parser.add_argument('-ti','--target_id', default=default_target_id,\
            help='Set target ID, default: ' + default_target_id)
    parser.add_argument('-ii','--initiator_id', default=default_initiator_id,\
            help='Set initiator ID, default: ' + default_initiator_id)
    parser.add_argument('-d', '--userdata_device_list', default=default_user_data_device_list,\
            help='Set user data device list, default: ' + default_user_data_device_list)
    parser.add_argument('-s', '--subsystem_count', default=default_subsystem_count,\
            help='Set subsystem count, default: ' + default_subsystem_count)
    parser.add_argument('-v', '--volume_count', default=default_volume_count,\
            help='Set volume count, default: ' + default_volume_count)
    parser.add_argument('-n', '--target_nic', default=default_target_nic,\
            help='Set target nic, default: ' + default_target_nic)
    parser.add_argument('-q', '--net_irq_cpulist', default=default_net_irq_cpulist,\
            help='Set net irq cpulist, default: ' + default_net_irq_cpulist)
    parser.add_argument('-o', '--transport', default=default_transport,\
            help='Set transport, default: ' + default_transport)
    parser.add_argument('-b', '--time_based', default=default_time_based,\
            help='Set time based, default: ' + default_time_based)
    parser.add_argument('--io_size', default=default_io_size,\
            help='Set io size, default: ' + default_io_size)
    parser.add_argument('--runtime', default=default_runtime,\
            help='Set runtime, default: ' + default_runtime)
    parser.add_argument('--ioat_qd', default=default_ioat_qd,\
            help='Set ioat_qd, default: ' + default_ioat_qd)
    parser.add_argument('--non_ioat_qd', default=default_non_ioat_qd,\
            help='Set non ioat_qd, default: ' + default_non_ioat_qd)
    parser.add_argument('--qd_128k', default=default_qd_128k,\
            help='Set qd for 128k, default: ' + default_qd_128k)
    parser.add_argument('--qd_4k', default=default_qd_4k,\
            help='Set qd for 4k, default: ' + default_qd_4k)
    parser.add_argument('-tr', '--test_revision', default=default_test_revision,\
            help='Set test revision, default: ' + default_test_revision)
    parser.add_argument('-tn', '--test_name', default=default_test_name,\
            help='Set test name, default: ' + default_test_name)
    parser.add_argument('-sw', '--sw_limit', default=default_sw_128k_limit,\
            help='Set sw 128k limit, default: ' + default_sw_128k_limit)
    parser.add_argument('-sr', '--sr_limit', default=default_sr_128k_limit,\
            help='Set sr 128k limit, default: ' + default_sr_128k_limit)
    parser.add_argument('-rw', '--rw_limit', default=default_rw_4k_limit,\
            help='Set rw 4k limit, default: ' + default_rw_4k_limit)
    parser.add_argument('-rr', '--rr_limit', default=default_rr_4k_limit,\
            help='Set rr 4k limit, default: ' + default_rr_4k_limit)

    global args
    args = parser.parse_args()
    print(args)

if __name__== "__main__":
    parse_argument()
    set_performance_config()
    try:
        bring_up_ibofos()
        execute_performance_test()
        convert_result_to_csv()
        terminate_pos()
    except subprocess.CalledProcessError:
        print("Failed to execute command")
        terminate_pos()
        sys.exit(-1)
    except Exception as e:
        print("Failed to exectue remote command", e)
        terminate_pos()
        sys.exit(-1)
    sys.exit(0)
