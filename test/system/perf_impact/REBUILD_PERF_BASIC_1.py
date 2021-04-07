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
LOG_PATH = "rebuild_perf_log"

EST_SECONDS_FOR_REBUILD = 250
SECONDS_PER_MINUTE = 30
MINUTES_PER_HOUR = 1
TESTTIME_IN_HOUR = 12

DEV_1 = "unvme-ns-0"
DEV_2 = "unvme-ns-1"
DEV_3 = "unvme-ns-2"
DEV_4 = "unvme-ns-3"

DEV_1_RECYCLED = "unvme-ns-4"
DEV_2_RECYCLED = "unvme-ns-5"
DEV_3_RECYCLED = "unvme-ns-6"
DEV_4_RECYCLED = "unvme-ns-7"

VOL_SIZE = 10 * ibofos_constant.SIZE_1GB
MAX_VOL_CNT = 2
VOL_NAME_PREFIX = "vol"

VOL_CNT = 0
ELAPSED_MIN = 0
ELAPSED_HOUR = 0

class TestTimer():
    def __init__(self, seconds, target):
        self._should_continue = False
        self.is_running = False
        self.seconds = seconds
        self.target = target
        self.thread = None

    def _handle_target(self):
        self.is_running = True
        self.target()
        self.is_running = False

        if (ELAPSED_HOUR == TESTTIME_IN_HOUR):
            self.cancel()
        else:
            self._start_timer()

    def _start_timer(self):
        if self._should_continue: # Code could have been running when cancel was called.
            self.thread = Timer(self.seconds, self._handle_target)
            self.thread.start()
            self.thread.join()
            write_log ("end test")

    def start(self):
        if not self._should_continue and not self.is_running:
            self._should_continue = True
            self._start_timer()
        else:
            write_log("Timer already started or running, please wait if you're restarting.")

    def cancel(self):
        if self.thread is not None:
            self._should_continue = False # Just in case thread is running and cancel fails.
            self.thread.cancel()
        else:
            write_log("Timer never started or failed to initialize.")

class FIO():
    def __init__(self):
        self._list = {}
        
    def start_fio(self, vol_id):
        key = VOL_NAME_PREFIX + str(vol_id)
        if key in self._list.keys():
            write_log ("fio id: " + key + " is already running")
            return False
        else:
            ip_addr = ibofos.TR_ADDR
            ns_id = str(vol_id + 1)
            test_name = key
            file_name = "trtype=tcp adrfam=IPv4 traddr=" + ip_addr + \
                " trsvcid=1158 subnqn=nqn.2019-04.ibof\:subsystem1 ns= " + ns_id
            remainig_min = (TESTTIME_IN_HOUR - ELAPSED_HOUR) * MINUTES_PER_HOUR - ELAPSED_MIN + EST_SECONDS_FOR_REBUILD
            runtime_sec = remainig_min * SECONDS_PER_MINUTE 
            write_log ("runtime_in_sec: " + str(runtime_sec))
            ioengine_path = IBOFOS_ROOT + "lib/spdk-19.10/examples/nvme/fio_plugin/fio_plugin"
            fio_proc = subprocess.Popen(["fio",
                "--ioengine=" + ioengine_path,\
                "--runtime=" + str(runtime_sec), \
                "--bs=4096", \
                "--iodepth=128",\
                "--readwrite=write",\
                "--offset=0",\
                "--bs_unaligned=1",\
                "--bs=4096",\
                "--verify=md5",\
                "--serialize_overlap=1",\
                "--time_based",\
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

def timer_tick():
    global ELAPSED_MIN
    ELAPSED_MIN = ELAPSED_MIN + 1
    if (ELAPSED_MIN == MINUTES_PER_HOUR):
        global ELAPSED_HOUR
        ELAPSED_HOUR = ELAPSED_HOUR + 1
        ELAPSED_MIN = 0
        write_log ("")
        write_log ("")
    print_time()

    if (ELAPSED_MIN == 0):
        tick_hour()

def print_time():
    curr = datetime.now()
    timelog = '[' + curr.strftime("%H:%M:%S") + '] Time elapsed ' + str(ELAPSED_HOUR).zfill(2) + ":" + str(ELAPSED_MIN).zfill(2)
    write_log(timelog)

def remove_log():
    if os.path.exists(LOG_PATH):
        os.remove(LOG_PATH)

def write_log(_log):
    print(_log)
    with open(LOG_PATH, "a") as result_file:
        result_file.write(_log+"\n")

def start_ibofos():
    write_log ("starting ibofos...")
    ibofos.start_ibofos()
    write_log ("ibofos is running")

def kill_ibofos():
    write_log ("killing ibofos...")
    fio_util.dispose()
    ibofos.kill_ibofos()
    write_log ("ibofos has been killed")

def scan_dev():
    write_log ("scan_dev begin")
    ibofos_util.pci_rescan()
    time.sleep(2)
    cli.scan_device()
    write_log(cli.list_device())
    write_log ("scan_dev done")

def create_array():
    DATA = DEV_1 + "," + DEV_2 + "," + DEV_3
    out = cli.create_array("uram0", DATA, DEV_4, "", "")
    write_log(out)
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("array created successfully")
        return True
    else:
        write_log ("array creation failed, code: " + str(code))
        return False

def mount_ibofos():
    out = cli.mount_ibofos()
    write_log(out)
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("array mounted successfully")
        return True
    else:
        write_log ("array mounting failed code: " + str(code))
        return False

def unmount_ibofos():
    out = cli.unmount_ibofos()
    write_log(out)
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
    out = out.replace('Create Volume !!!!', '')
    write_log(out)
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
    write_log(out)
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("volume: " + vol_name + " mounted successfully")
        return True
    else:
        write_log ("volume: " + vol_name + " mounting failed")
        return False

def detach_data(target):
    ibofos_util.pci_detach_and_attach(target)
    time.sleep(0.1)

def add_spare(spare):
    out = cli.add_device(spare, "")
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("Spare device: " + spare + " has been added successfully")
        return True
    else:
        write_log ("Spare device: " + spare + " adding failed, code: " + str(code))
        return False

def tick_hour():
    ret = do_event(ELAPSED_HOUR)
    if ret == False:
        print ("TEST FAILED")
        kill_ibofos()

def init_test():
    scan_dev()
    create_array()
    ret = mount_ibofos()
    return ret

def add_new_vol_and_do_io(cnt = 1):
    for i in range(cnt):
        ret = create_and_mount_vol()
        if ret == False:
            write_log ("failed to add volume")
            return False
        fio_util.start_fio(VOL_CNT - 1)
        write_log (VOL_NAME_PREFIX + str(VOL_CNT - 1) + " volume is newly created and I/O is being performed there")
    return True

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

def wait_until_rebuild_start():
    while check_situation("REBUILDING") == False:
        time.sleep(1)
    return True

def wait_until_rebuild_done():
    while check_situation("REBUILDING") == True:
        time.sleep(1)
    return True

def do_event(elapsed_hour):
    if elapsed_hour == 0:
        ret = init_test()
        if ret == True:
            for i in range(MAX_VOL_CNT):
                if add_new_vol_and_do_io() == False:
                    return False
                time.sleep(1)
            return check_state("NORMAL")
        return False

    elif elapsed_hour >= 1 and elapsed_hour <= 11:
        return check_state("NORMAL")

    elif elapsed_hour == 12:
        print (cli.array_info(""))
        detach_data(DEV_1)
        if wait_until_rebuild_start() == True:
            rebuild_start_time = datetime.now()
            if wait_until_rebuild_done() == True:
                duration = datetime.now() - rebuild_start_time
                write_log ("rebuild_duration(sec):" + str(duration.total_seconds()))
        kill_ibofos()
        return True
 
    else:
        write_log ("unaddressed timing")
        return False

def execute(ip_addr, perf_impact):
    global VOL_CNT
    VOL_CNT = 0
    global ELAPSED_MIN
    ELAPSED_MIN = 0
    global ELAPSED_HOUR
    ELAPSED_HOUR = 0
    
    write_log("SECONDS PER MIN: " + str(SECONDS_PER_MINUTE))
    write_log("MINUTES PER HOUR: " + str(MINUTES_PER_HOUR))
    ibofos.set_addr(ip_addr)
    write_log("IPADDRESS: " + ibofos.TR_ADDR)
    write_log("PERF IMPACT: " + perf_impact)
    print_time()
    ibofos_util.kill_process("ibofos")
    start_ibofos()
    write_log(cli.update_event_qos("rebuild", perf_impact))
    ret = do_event(0)
    if ret == True:
        t = TestTimer(SECONDS_PER_MINUTE, timer_tick)
        t.start()
        
    else:
        write_log ("TEST FAILED WHILE INITIALIZATION")
        kill_ibofos()
# Example Usage
if __name__ == "__main__":
    execute(sys.argv[1], sys.argv[2])
