#!/usr/bin/env python3
from threading import Timer
from datetime import datetime

import subprocess
import os
import sys
sys.path.append("../lib/")
import json
import json_parser
import ibofos
import ibofos_util
import cli
import ibofos_constant
import time

IBOFOS_ROOT = '../../../'
#TEST_TIME = 60
TEST_SIZE = 256 * 1024 * 1024

TEST_SIZE = TEST_SIZE * 2

RC = True

DEV_1 = "unvme-ns-0"
DEV_2 = "unvme-ns-1"
DEV_3 = "unvme-ns-2"
DEV_4 = "unvme-ns-3"

VOL_SIZE = 5 * ibofos_constant.SIZE_1GB

MAX_VOL_CNT = 1

VOL_NAME_PREFIX = "vol"
VOL_CNT = 0

PREVIOUS_TIME = "0"
START_GC = False

class FIO():
    def __init__(self):
        self._list = {}
        
    def start_fio(self, vol_id, workload, io_size_bytes):
        key = VOL_NAME_PREFIX + str(vol_id)
        ip_addr = ibofos.TR_ADDR
        ns_id = str(vol_id + 1)
        test_name = key
        file_name = "trtype=tcp adrfam=IPv4 traddr=" + ip_addr + \
            " trsvcid=1158 subnqn=nqn.2019-04.ibof\:subsystem1 ns= " + ns_id
        ioengine_path = IBOFOS_ROOT + "lib/spdk-19.10/examples/nvme/fio_plugin/fio_plugin"
        fio_proc = subprocess.Popen(["fio",
            "--ioengine=" + ioengine_path,\
            "--bs=4096", \
            "--iodepth=128",\
            "--readwrite=" + workload,\
            "--offset=0",\
            "--io_size=" + str(io_size_bytes),\
            "--bs_unaligned=1",\
            "--bs=4096",\
            "--verify=0",\
            "--serialize_overlap=1",\
            "--numjobs=1",\
            "--thread=1",\
            "--group_reporting=1",\
            "--direct=1",\
            "--name=" + test_name, \
            "--filename=" + file_name]\
            )

        self._list[key] = fio_proc
        write_log ("fio id: " + key + " has been started")
        return True

    def stop_fio(self, key):
        if key in self._list.keys():
            fio_proc = self._list[key]
            if fio_proc.poll() is None:
                fio_proc.kill()
                fio_proc.wait()
                write_log ("fio id: " + key + " has been terminated")
            else:
                write_log ("fio id: " + key + " already been terminated")

    def dispose(self):
        write_log ("fio dispose, dict len: " + str(len(self._list)))
        for key in self._list:
            write_log ("stop_fio target: " + key)
            self.stop_fio(key)
        self._list.clear()

fio_util = FIO()

def write_log(_log):
    print(_log)
    with open("gc_trigger_basic_test_log", "a") as result_file:
        result_file.write(_log+"\n")

def start_ibofos():
    write_log ("starting ibofos...")
    ibofos.start_ibofos()
    write_log ("ibofos is running")

def exit_ibofos():
    write_log ("exiting ibofos...")
    if get_state() != "OFFLINE":
        ret = unmount_ibofos()
        if ret == False:
            write_log("ibofos unmounting failed")
            return False
    fio_util.dispose()
    ibofos.exit_ibofos()
    write_log ("ibofos has been terminated")
    return True

def kill_ibofos():
    write_log ("killing ibofos...")
    ibofos.kill_ibofos()
    write_log ("ibofos has been killed")

def restart_ibofos():
    ret = exit_ibofos()
    if ret == False:
        write_log("ibofos restarting failed while exiting")
        return False
    start_ibofos()
    write_log("ibofos has been restarted")
    return True

def scan_dev():
    write_log ("scan_dev begin")
    ibofos_util.pci_rescan()
    time.sleep(2)
    cli.scan_device()
    cli.list_device()
    write_log ("scan_dev done")

def create_array():
    DATA = DEV_1 + "," + DEV_2 + "," + DEV_3
    out = cli.create_array("uram0", DATA, "", "", "")
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("array created successfully")
        return True
    else:
        write_log ("array creation failed, code: " + str(code))
        return False

def load_array():
    out = cli.load_array("")
    code = json_parser.get_response_code(out)
    cli.list_device()
    cli.list_volume("")
    if code == 0:
        write_log ("array loaded successfully")
        return True
    else:
        write_log ("array loading failed, code: " + str(code))
        return False

def mount_ibofos():
    out = cli.mount_ibofos()
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("array mounted successfully")
        return True
    else:
        write_log ("array mounting failed code: " + str(code))
        return False

def unmount_ibofos():
    out = cli.unmount_ibofos()
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("array unmounted successfully")
        return True
    else:
        write_log ("array unmounting failed code: " + str(code))
        return False

def create_and_mount_vol():
    global VOL_CNT
    vol_name = VOL_NAME_PREFIX + str(VOL_CNT)
    write_log ("try to create volume, name: " + vol_name + ", size: " + str(VOL_SIZE))
    out = cli.create_volume(vol_name, str(VOL_SIZE), "", "", "")
    code = json_parser.get_response_code(out)
    if code == 0:
        VOL_CNT = VOL_CNT + 1
        write_log ("volume: " + vol_name + " created successfully, vol_cnt: " + str(VOL_CNT))
        return mount_vol(vol_name)
    else:
        write_log ("volume: " + vol_name + " creation failed, code: " + str(code))
        return False

def mount_vol(vol_name):
    out = cli.mount_volume(vol_name, "", "")
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("volume: " + vol_name + " mounted successfully")
        return True
    else:
        write_log ("volume: " + vol_name + " mounting failed, code: " + str(code))
        return False

def init_test():
    #ibofos_util.kill_process("ibofos")
    start_ibofos()
    scan_dev()
    create_array()
    ret = mount_ibofos()
    return ret

def add_new_vol(cnt = 1):
    for i in range(cnt):
        ret = create_and_mount_vol()
        if ret == False:
            write_log ("failed to add volume")
            return False
        write_log("success to add volume")
    return True

def do_io(cnt = 1, workload = "write", io_size_bytes = TEST_SIZE):
    for i in range(cnt):
        fio_util.start_fio(i, workload, io_size_bytes)
    write_log (VOL_NAME_PREFIX + str(VOL_CNT - 1) + " volume is newly created and I/O is being performed there")
    return True

def forced_trigger_gc():
    global PREVIOUS_TIME
    out = cli.wbt_request("get_gc_status","")
    data = json.loads(out)
    start_time = data['Response']['result']['data']['gc']['time']['start']
    PREVIOUS_TIME = start_time
    write_log ("gc trigger, previous time: " + start_time)

    time.sleep(1)
    cli.wbt_request("do_gc","")
    return True

def check_gc_done():
    global PREVIOUS_TIME
    global START_GC
    if START_GC == False:
        out = cli.wbt_request("get_gc_status","")
        data = json.loads(out)
        start_time = data['Response']['result']['data']['gc']['time']['start']
        if start_time == PREVIOUS_TIME:
            write_log ("not gc started yet, previous time: " + start_time)
            return False
        START_GC = True
        PREVIOUS_TIME = start_time
    out = cli.wbt_request("get_gc_status","")
    data = json.loads(out)
    gc_status = data['Response']['result']['data']['gc']['status']['active']
    if gc_status == "done":
        write_log ("gc done")
        return True
    write_log ("gc start but wait gc done, current status is " + gc_status)
    return False

def get_situation():
    out = cli.get_ibofos_info()
    data = json.loads(out)
    situ = data['Response']['info']['situation']
    return situ

def check_situation(situ_expected):
    situ = get_situation()
    if situ == situ_expected:
        write_log ("current situation is " + situ)
        return True
    write_log ("current situation is " + situ + " but we expected " + situ_expected)
    return False

def get_state():
    out = cli.get_ibofos_info()
    data = json.loads(out)
    state = data['Response']['info']['state']
    return state

def check_state(state_expected):
    state = get_state()
    if state == state_expected:
        write_log ("current state is " + state)
        return True
    write_log ("current state is " + state + " but we expected " + state_expected)
    return False

def do_event(request_event):
    if request_event == "INIT":
        write_log ("init test")
        ret = init_test()
        if ret == False:
            write_log ("failed to initiate ibofos")
            return False
        write_log ("add new vol")
        if add_new_vol(MAX_VOL_CNT) == True:
            return check_state("NORMAL")
        return False
    elif request_event == "BASE_WRITE":
        write_log ("start base write")
        do_io(MAX_VOL_CNT, "write", TEST_SIZE)
        time.sleep(3)
        write_log ("end base write")
        return True
    elif request_event == "OVER_WRITE":
        write_log ("start over write")
        do_io(MAX_VOL_CNT, "randwrite", TEST_SIZE / 2)
        time.sleep(3)
        write_log ("end over write")
        return True
    elif request_event == "GC_TRIGGER":
        write_log ("start gc trigger")
        ret = forced_trigger_gc()
        write_log ("end gc trigger")
        return ret
    elif request_event == "WAIT_GC_DONE":
        ret = check_gc_done()
        return ret
    elif request_event == "EXIT_TEST":
        exit_ibofos()
        return True
    else:
        write_log ("unaddressed timing")
        return False

def main(ip_addr):
    ibofos.set_addr(ip_addr)
    write_log("IPADDRESS: " + ibofos.TR_ADDR)
    write_log ("init start")
    ret = do_event("INIT")
    if ret == False:
        write_log ("init fail")
        return False
    write_log ("base write")
    ret = do_event("BASE_WRITE")
    if ret == False:
        write_log ("write fail")
        return False
    write_log ("over write")
    ret = do_event("OVER_WRITE")
    if ret == False:
        write_log ("over write fail")
        return False
    write_log ("gc trigger")
    ret = do_event("GC_TRIGGER")
    if ret == False:
        write_log ("gc trigger fail")
        return False
    write_log ("wait gc done")
    ret = False
    retry_count = 0
    while ret == False:
        retry_count += 1
        if retry_count == 60:
            write_log ("too rate.. something problem")
            return False
        time.sleep(2)
        ret = do_event("WAIT_GC_DONE")
    write_log ("gc done")
    ret = do_event("EXIT_TEST")
    if ret == False:
        return False

if __name__ == "__main__":
    write_log("============== START GC BASIC 1 TEST ==============")
    #write_log("TEST TIME(s): " + str(TEST_TIME))
    main(sys.argv[1])
