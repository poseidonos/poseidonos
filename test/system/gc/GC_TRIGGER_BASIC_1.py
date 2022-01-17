#!/usr/bin/env python3
from threading import Timer
from datetime import datetime

import subprocess
import os
import sys
sys.path.append("../lib/")
import json
import json_parser
import pos
import pos_util
import cli
import pos_constant
import time

POS_ROOT = '../../../'
#TEST_TIME = 60
TEST_SIZE = 256 * 1024 * 1024

TEST_SIZE = TEST_SIZE * 2

RC = True

DEV_1 = "unvme-ns-0"
DEV_2 = "unvme-ns-1"
DEV_3 = "unvme-ns-2"
DEV_4 = "unvme-ns-3"

VOL_SIZE = 5 * pos_constant.SIZE_1GB

MAX_VOL_CNT = 1

VOL_NAME_PREFIX = "vol"
VOL_CNT = 0

PREVIOUS_TIME = "0"
START_GC = False

ARRAYNAME = "POSArray"

class FIO():
    def __init__(self):
        self._list = {}
        
    def start_fio(self, vol_id, workload, io_size_bytes):
        key = VOL_NAME_PREFIX + str(vol_id)
        ip_addr = pos.TR_ADDR
        ns_id = str(vol_id + 1)
        test_name = key
        file_name = "trtype=tcp adrfam=IPv4 traddr=" + ip_addr + \
            " trsvcid=1158 subnqn=nqn.2019-04.pos\:subsystem1 ns= " + ns_id
        ioengine_path = POS_ROOT + "lib/spdk/examples/nvme/fio_plugin/fio_plugin"
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

def start_pos():
    write_log ("starting pos...")
    ret = pos.start_pos()
    if ret is False:
        write_log("faild to start pos")
        return False
    write_log("pos is running")
    return True

def exit_pos():
    write_log ("exiting pos...")
    state = get_state();
    if state == "NORMAL" or state == "BUSY":
        ret = unmount_pos()
        if ret == False:
            write_log("pos unmounting failed")
            return False
    fio_util.dispose()
    pos.exit_pos()
    write_log ("pos has been terminated")
    return True

def kill_pos():
    write_log ("killing pos...")
    pos.kill_pos()
    write_log ("pos has been killed")

def restart_pos():
    ret = exit_pos()
    if ret == False:
        write_log("pos restarting failed while exiting")
        return False
    ret = start_pos()
    if ret is False:
        write_log("pos restarting failed while starting")
        return False
    write_log("pos has been restarted")
    return True

def scan_dev():
    write_log ("scan_dev begin")
    pos_util.pci_rescan()
    time.sleep(2)
    cli.scan_device()
    cli.list_device()
    write_log ("scan_dev done")

def create_array():
    DATA = DEV_1 + "," + DEV_2 + "," + DEV_3
    out = cli.create_array("uram0", DATA, "", ARRAYNAME, "")
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("array created successfully")
        return True
    else:
        write_log ("array creation failed, code: " + str(code))
        return False

def mount_pos():
    out = cli.mount_array(ARRAYNAME)
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("array mounted successfully")
        return True
    else:
        write_log ("array mounting failed code: " + str(code))
        return False

def unmount_pos():
    out = cli.unmount_array(ARRAYNAME)
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
    out = cli.create_volume(vol_name, str(VOL_SIZE), "", "", ARRAYNAME)
    code = json_parser.get_response_code(out)
    if code == 0:
        VOL_CNT = VOL_CNT + 1
        write_log ("volume: " + vol_name + " created successfully, vol_cnt: " + str(VOL_CNT))
        return mount_vol(vol_name)
    else:
        write_log ("volume: " + vol_name + " creation failed, code: " + str(code))
        return False

def mount_vol(vol_name):
    out = cli.mount_volume(vol_name, ARRAYNAME, "")
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("volume: " + vol_name + " mounted successfully")
        return True
    else:
        write_log ("volume: " + vol_name + " mounting failed, code: " + str(code))
        return False

def mbr_reset():
    cli.mbr_reset()

def init_test():
    ret = start_pos()
    if ret is False:
        return False
    scan_dev()
    mbr_reset()
    create_array()
    ret = mount_pos()
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

def get_state():
    out = cli.array_info(ARRAYNAME)
    state = json_parser.get_state(out)
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
            write_log ("failed to initiate pos")
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
        exit_pos()
        return True
    else:
        write_log ("unaddressed timing")
        return False

def main(ip_addr):
    pos.set_addr(ip_addr)
    write_log("IPADDRESS: " + pos.TR_ADDR)
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
