#!/usr/bin/env python3

import sys
import os

sys.path.append("../lib/")
sys.path.append("../io_path/")

from datetime import datetime
import subprocess
import psutil
import signal

import ibofos
import ibofos_constant

import cli
import json_parser
import spdk_rpc

import TEST
import TEST_LOG
import TEST_DEBUGGING

######################################################################################
isIbofExecuted = False
######################################################################################

def chldSignal_handler(sig, frame):
    global isIbofExecuted
    if isIbofExecuted == True:
        isAllive = ibof_proc.poll()
        if isAllive != None:
            TEST_LOG.print_err("POS terminated unexpectedly")
            isIbofExecuted = False
            sys.exit(1)

def start_ibofos():
    global ibof_proc
    global isIbofExecuted

    TEST_LOG.print_info("* Starting POS")
    ibof_execution = TEST.ibof_root + "bin/ibofos"

    with open(TEST.ibof_log_path, "a") as log_file:
        ibof_proc = subprocess.Popen(ibof_execution, stdout=log_file, stderr=log_file)

    isIbofExecuted = True

def shutdown_ibofos():
    global isIbofExecuted
    global ibof_proc
    
    TEST_LOG.print_info("* Exiting POS")
    out = cli.unmount_ibofos()
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to unmount ibofos")
        TEST_LOG.print_debug(out)
        sys.exit(1)
    out = cli.exit_ibofos()
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to exit ibofos")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    ibof_proc.wait()
    isIbofExecuted = False
    
def kill_ibofos():
    global isIbofExecuted
    global ibof_proc

    isIbofExecuted = False
    ibof_proc.kill()
    ibof_proc.wait()
    TEST_LOG.print_info("* POS killed")

def clean_bringup():
    TEST_LOG.print_info("* POS clean bringup")

    if (os.path.isfile(TEST.mockfile)):
        os.remove(TEST.mockfile)

    start_ibofos()
    subprocess.call(["sleep", "3"])

    setup()
    create_array()
    mount_array()

    TEST_LOG.print_info("* Fininshed bringup")

def dirty_bringup(dump=0):
    TEST_LOG.print_info("* POS dirty bringup")

    start_ibofos()
    subprocess.call(["sleep", "3"])

    setup()
    load_array()
    mount_array()

    TEST_DEBUGGING.dump_journal("LogBuffer_AfterSPO")
    TEST_LOG.print_info("* Fininshed bringup")

def setup():
    spdk_rpc.send_request("nvmf_create_transport -t " + TEST.trtype + " -b 64 -n 4096")
    spdk_rpc.send_request("bdev_malloc_create -b uram0 1024 512")

    out = cli.scan_device()
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to scan device")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Setup POS")

def create_array():
    out = cli.create_array("uram0", "unvme-ns-0,unvme-ns-1,unvme-ns-2", "unvme-ns-3", "", "")
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to create array")
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* Array created")

def load_array():
    out = cli.load_array("")
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to load array")
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* Array loaded")

def mount_array():
    out = cli.mount_ibofos()
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to mount ibofos")
        TEST_LOG.print_debug(out)
        sys.exit(1)
    TEST_LOG.print_info("* POS mounted")

def create_subsystem(volumeId):
    out = spdk_rpc.send_request("nvmf_create_subsystem " + TEST.NQN + str(volumeId) \
        + " -a -s IBOF0000000000000" + str(volumeId) + " -d IBOF_VOLUME_EXTENTION")
    if out != 0:
        TEST_LOG.print_err("Failed to create subsystem")
        sys.exit(1)

def create_volume(volumeId, subnqn=""):
    out = cli.create_volume("vol"+str(volumeId), str(TEST.volSize), "0", "0", "")
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to create volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} created".format(volumeId))
    mount_volume(volumeId)

def mount_volume(volumeId):
    out = spdk_rpc.send_request("nvmf_subsystem_add_listener " + TEST.NQN + str(volumeId) \
        + " -t " + TEST.trtype + " -a " + str(TEST.traddr) + " -s " + str(TEST.port))
    if out != 0:
        TEST_LOG.print_err("Failed to create volume")
        sys.exit(1)
    
    out = cli.mount_volume("vol" + str(volumeId), "", TEST.NQN + str(volumeId))
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to mount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} mounted".format(volumeId))

def unmount_volume(volumeId):
    out = cli.unmount_volume("vol" + str(volumeId), "")
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to unmount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    out = spdk_rpc.send_request("nvmf_subsystem_remove_listener " + TEST.NQN + str(volumeId) \
        + " -t " + TEST.trtype + " -a " + str(TEST.traddr) + " -s " + str(TEST.port))
    if out != 0:
        TEST_LOG.print_err("Failed to unmount volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} unmounted".format(volumeId))

def delete_volume(volumeId):
    out = cli.delete_volume("vol" + str(volumeId), "")
    ret = json_parser.get_response_code(out)
    if ret != 0:
        TEST_LOG.print_err("Failed to delete volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    out = spdk_rpc.send_request("nvmf_delete_subsystem " + TEST.NQN + str(volumeId))
    if out != 0:
        TEST_LOG.print_err("Failed to delete volume")
        TEST_LOG.print_debug(out)
        sys.exit(1)

    TEST_LOG.print_info("* Volume {} deleted".format(volumeId))

def backup_nvram():
    backup_execution = TEST.ibof_root + "script/backup_latest_hugepages_for_uram.sh"
    with open(TEST.output_log_path, "a") as log_file:
        backup_proc = subprocess.Popen([backup_execution], stdout=log_file, stderr=log_file)

    if backup_proc.wait() != 0:
        TEST_LOG.print_err("Failed to backup unvram")
        sys.exit(1)

def trigger_spor():
    TEST_DEBUGGING.dump_journal("LogBuffer_BeforeSPO")
    TEST_DEBUGGING.flush_gcov()

    kill_ibofos()
    kill_process("fio")
    backup_nvram()

def kill_process(procname, sig=9):
    for proc in psutil.process_iter():
        try:
            if procname in proc.name():
                proc.send_signal(sig)
                proc.wait()
                TEST_LOG.print_info("* " + procname + " killed")
        except psutil.NoSuchProcess:
            pass

def cleanup_process():
    os.system('rm -rf /dev/shm/ibof_nvmf_trace*')

    global isIbofExecuted
    isIbofExecuted = False
    kill_process("ibofos")
    kill_process("fio")

def cleanup_ibof_logs():
    os.system('rm -rf /etc/ibofos/core/*')
    os.system('rm -rf /var/log/ibofos/*')