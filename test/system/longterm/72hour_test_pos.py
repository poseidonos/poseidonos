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
ARRAYNAME = "POSArray"

#SECONDS FOR A MINUTE
#SET 60 FOR REALTIME TESTING
SECONDS_PER_MINUTE = 60
#MINUTES FOR AN HOUR
#SET 60 FOR REALTIME TESTING
MINUTES_PER_HOUR = 60
#DELAY FOR REBUIDING 
REBUILDING_DELAY_MIN = 0

TESTTIME_IN_HOUR = 72
ELAPSED_MIN = 0
ELAPSED_HOUR = 0
RC = True

DEV_PREFIX = "unvme-ns-"
SYS_DEV_CNT = 32
INIT_DATA_DEV_CNT = 31
INIT_SPARE_DEV_CNT = 1
DETACHED_CNT = 0

VOL_SIZE = 100 * pos_constant.SIZE_1GB

#MAX VOLUME COUNT FOR A TEST
#In the VM environment recommends up to 12 or less
MAX_VOL_CNT = 36

VOL_NAME_PREFIX = "vol"
VOL_CNT = 0
MOUNTED_VOL_CNT = 0
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

        if (RC == False):
            self.cancel()
        elif (ELAPSED_HOUR == TESTTIME_IN_HOUR):
            self.cancel()
        else:
            self._start_timer()

    def _start_timer(self):
        if self._should_continue: # Code could have been running when cancel was called.
            self.thread = Timer(self.seconds, self._handle_target)
            self.thread.start()

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

    def join(self):
        while self._should_continue == True:
            time.sleep(1)

class FIO():
    def __init__(self):
        self._list = {}
        
    def start_fio(self, vol_id):
        key = VOL_NAME_PREFIX + str(vol_id)
        if key in self._list.keys():
            write_log ("fio id: " + key + " is already running")
            return False
        else:
            ip_addr = pos.TR_ADDR
            ns_id = str(vol_id + 1)
            test_name = key
            file_name = "trtype=tcp adrfam=IPv4 traddr=" + ip_addr + \
                " trsvcid=1158 subnqn=nqn.2019-04.pos\:subsystem1 ns= " + ns_id
            remainig_min = (TESTTIME_IN_HOUR - ELAPSED_HOUR) * MINUTES_PER_HOUR - ELAPSED_MIN
            runtime_sec = remainig_min * SECONDS_PER_MINUTE
            write_log ("runtime_in_sec: " + str(runtime_sec))
            ioengine_path = POS_ROOT + "lib/spdk/examples/nvme/fio_plugin/fio_plugin"
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
    if (ELAPSED_MIN == MINUTES_PER_HOUR + REBUILDING_DELAY_MIN):
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

def write_log(_log):
    print(_log)
    with open("72hour_test_log", "a") as result_file:
        result_file.write(_log+"\n")

def start_pos():
    write_log ("starting pos...")
    ret = pos.start_pos()
    if ret is False:
        write_log("faild to start pos")
        return False
    write_log ("pos is running")
    return True

def exit_pos():
    write_log ("exiting pos...")
    state = get_state()
    if state == "NORMAL" or state == "BUSY":
        ret = unmount_pos()
        if ret == False:
            write_log("pos unmounting failed")
            return False
    fio_util.dispose()
    pos.exit_pos()
    write_log ("pos has been terminated")
    return True

def abort_pos():
    write_log ("abort pos for dump...")
    fio_util.dispose()
    pos_util.abort_process("poseidonos")
    write_log ("pos has been terminated")

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
    write_log(cli.list_device())
    write_log ("scan_dev done")

def create_array():
    DATA = ""
    SPARE = ""
    for i in range(INIT_DATA_DEV_CNT):
        devname = DEV_PREFIX + str(i)
        DATA = DATA + devname
        if i < INIT_DATA_DEV_CNT - 1:
            DATA = DATA + ","
    
    for i in range(INIT_DATA_DEV_CNT, INIT_DATA_DEV_CNT+INIT_SPARE_DEV_CNT):
        devname = DEV_PREFIX + str(i)
        SPARE = SPARE + devname
        if i < INIT_DATA_DEV_CNT + INIT_SPARE_DEV_CNT - 1:
            SPARE = SPARE + ","

    out = cli.create_array("uram0", DATA, SPARE, ARRAYNAME, "")
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
        global MOUNTED_VOL_CNT
        MOUNTED_VOL_CNT = 0
        write_log ("array unmounted successfully")
        return True
    else:
        write_log ("array unmounting failed code: " + str(code))
        return False

def create_and_mount_vol():
    global VOL_CNT
    
    if VOL_CNT == MAX_VOL_CNT :
        return True #ignore as true

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
        global MOUNTED_VOL_CNT
        MOUNTED_VOL_CNT = MOUNTED_VOL_CNT + 1
        write_log ("volume: " + vol_name + " mounted successfully")
        return True
    else:
        write_log ("volume: " + vol_name + " mounting failed, code: " + str(code))
        return False

def umnmount_vol(vol_name):
    out = cli.unmount_volume(vol_name, ARRAYNAME)
    code = json_parser.get_response_code(out)
    if code == 0:
        global MOUNTED_VOL_CNT
        MOUNTED_VOL_CNT = MOUNTED_VOL_CNT - 1
        write_log ("volume: " + vol_name + " unmounted successfully")
        return True
    else:
        write_log ("volume: " + vol_name + " unmounting failed, code: " + str(code))
        return False

def detach_data(target):
    pos_util.pci_detach_and_attach(target)
    time.sleep(0.1)
    global DETACHED_CNT
    DETACHED_CNT = DETACHED_CNT + 1

def add_spare(spare):
    write_log("add_spare : " + spare)
    out = cli.add_device(spare, ARRAYNAME)
    code = json_parser.get_response_code(out)
    if code == 0:
        write_log ("Spare device: " + spare + " has been added successfully")
        return True
    else:
        write_log ("Spare device: " + spare + " adding failed, code: " + str(code))
        return False

def mbr_reset():
    cli.mbr_reset()

def tick_hour():
    global RC
    RC = do_event(ELAPSED_HOUR)

def init_test():
    # pos_util.kill_process("poseidonos")
    ret = start_pos()
    if ret is False:
        return False
    scan_dev()
    mbr_reset()
    create_array()
    ret = mount_pos()
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

def unmount_vols(cnt = 1):
    for i in range(cnt):
        if MOUNTED_VOL_CNT == 0 :
            return True #ignore as true
        vol_name = VOL_NAME_PREFIX + str(MOUNTED_VOL_CNT - 1)
        ret = umnmount_vol(vol_name)
        if ret == False:
            write_log ("failed to unmount volume")
            return False
        fio_util.stop_fio(MOUNTED_VOL_CNT)
    return True

def mount_vols_and_do_io(cnt = 1):
    for i in range(cnt):
        write_log("mount_vols_and_do_io (" + str(MOUNTED_VOL_CNT) + "/" + str(VOL_CNT) + ")")
        if MOUNTED_VOL_CNT == VOL_CNT :
            return True #ignore as true
        vol_name = VOL_NAME_PREFIX + str(MOUNTED_VOL_CNT)
        ret = mount_vol(vol_name)
        if ret == False:
            write_log ("failed to unmount volume")
            return False
        fio_util.start_fio(MOUNTED_VOL_CNT - 1)
        write_log (vol_name + " is mounted and I/O is being performed there")
    return True

def get_situation():
    out = cli.array_info(ARRAYNAME)
    situ = json_parser.get_situation(out)
    return situ

def check_situation(situ_expected):
    situ = get_situation()
    if situ == situ_expected:
        write_log ("current situation is " + situ)
        return True
    write_log ("current situation is " + situ + " but we expected " + situ_expected)
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

def get_detach_target_datadev():
    return DEV_PREFIX + str(DETACHED_CNT)

def get_system_dev_for_spare():
    devicelist = cli.list_device()
    write_log(devicelist)
    data = json.loads(devicelist)
    actual_list = []
    for item in data['Response']['result']['data']['devicelist'] :
        if item['type'] == "SSD" and item['class'] == "SYSTEM" :
            return item['name']
    return "no remaining system device"

def do_event(elapsed_hour):
    global REBUILDING_DELAY_MIN
    write_log(cli.list_volume(ARRAYNAME))
    if elapsed_hour == 0:
        ret = init_test()
        if ret == True:
            if add_new_vol_and_do_io() == True:
                return check_state("NORMAL")
        return False

    elif elapsed_hour >= 1 and elapsed_hour <= 5:
        if add_new_vol_and_do_io() == True:
            return check_state("NORMAL")
        return False

    elif elapsed_hour == 6:
        ret = restart_pos()
        if ret == True:
            scan_dev()
            if mount_pos() == True:
                if mount_vols_and_do_io(VOL_CNT) == False:
                    return False
                if add_new_vol_and_do_io() == True:
                    return check_state("NORMAL")
        return False
    
    elif elapsed_hour >= 7 and elapsed_hour <= 11:
        if add_new_vol_and_do_io() == True:
            return check_state("NORMAL")
        return False

    elif elapsed_hour == 12:
        ret = restart_pos()
        if ret == True:
            scan_dev()
            if mount_pos() == True:
                if mount_vols_and_do_io(VOL_CNT) == False:
                    return False
                if add_new_vol_and_do_io(8) == True:
                    detach_data(get_detach_target_datadev())
                    time.sleep(60)
                    REBUILDING_DELAY_MIN = 2
                    return check_situation("REBUILDING")
        return False

    elif elapsed_hour >= 13 and elapsed_hour <= 17:
        return check_state("NORMAL") or check_situation("REBUILDING")

    elif elapsed_hour >= 18 and elapsed_hour <= 23:
        REBUILDING_DELAY_MIN = 0
        if check_situation("REBUILDING") == True:
            write_log("rebuilding time out")
        return check_state("NORMAL")

    elif elapsed_hour == 24:
        detach_data(get_detach_target_datadev())
        time.sleep(60)
        return check_situation("DEGRADED")

    elif elapsed_hour >= 25 and elapsed_hour <= 35:
        if add_new_vol_and_do_io() == True:
            return check_situation("DEGRADED")
        return False

    elif elapsed_hour == 36:
        ret = restart_pos()
        if ret == True:
            scan_dev()
            if mount_pos() == True:
                if mount_vols_and_do_io(VOL_CNT) == True:
                    return check_situation("DEGRADED")
        return False

    elif elapsed_hour == 37:
        return check_situation("DEGRADED")

    elif elapsed_hour == 38:
        ret = add_spare(get_system_dev_for_spare())
        time.sleep(60)
        if ret == True:
            REBUILDING_DELAY_MIN = 2
            if check_situation("REBUILDING") == True:
                ret = add_spare(get_system_dev_for_spare())
                return ret
        return False

    elif elapsed_hour >= 39 and elapsed_hour <= 43:
        return check_state("NORMAL") or check_situation("REBUILDING")

    elif elapsed_hour == 44:
        REBUILDING_DELAY_MIN = 0
        if check_situation("REBUILDING") == True:
            write_log("rebuilding time out")
        return check_state("NORMAL")

    elif elapsed_hour >= 45 and elapsed_hour <= 49:
        return check_state("NORMAL")
    
    elif elapsed_hour == 50:
        detach_data(get_detach_target_datadev())
        time.sleep(60)
        REBUILDING_DELAY_MIN = 2
        ret = check_situation("REBUILDING")
        if ret == True:
            return unmount_vols(12)

    elif elapsed_hour == 51:
        ret = unmount_vols(12)
        if ret == True:
            return check_state("NORMAL") or check_situation("REBUILDING")
        return False

    elif elapsed_hour == 52:
        ret = unmount_vols(6)
        if ret == True:
            return check_state("NORMAL") or check_situation("REBUILDING")
        return False

    elif elapsed_hour == 53:
        ret = unmount_vols(3)
        if ret == True:
            return check_state("NORMAL") or check_situation("REBUILDING")
        return False

    elif elapsed_hour == 54:
        ret = unmount_vols(3)
        if ret == True:
            return check_state("NORMAL") or check_situation("REBUILDING")
        return False

    elif elapsed_hour == 55:
        return check_state("NORMAL") or check_situation("REBUILDING")

    elif elapsed_hour == 56:
        REBUILDING_DELAY_MIN = 0
        if check_state("NORMAL") == True:
            return mount_vols_and_do_io(3)
        return False

    elif elapsed_hour == 57:
        if check_state("NORMAL") == True:
            return mount_vols_and_do_io(3)
        return False

    elif elapsed_hour == 58:
        if check_state("NORMAL") == True:
            return mount_vols_and_do_io(6)
        return False

    elif elapsed_hour == 59:
        if check_state("NORMAL") == True:
            return mount_vols_and_do_io(12)
        return False

    elif elapsed_hour == 60:
        ret = add_spare(get_system_dev_for_spare())
        if ret == True:
            ret = unmount_vols()
            if ret == True:
                return check_situation("NORMAL")
        return False

    elif elapsed_hour >= 61 and elapsed_hour <= 71:
        ret = unmount_vols()
        if ret == True:
            return check_situation("NORMAL") or check_situation("REBUILDING")
        return False

    elif elapsed_hour == TESTTIME_IN_HOUR:
        return True
    
    else:
        write_log ("unaddressed timing")
        return False

def main(ip_addr):
    pos.set_addr(ip_addr)
    write_log("IPADDRESS: " + pos.TR_ADDR)
    print_time()
    global RC
    RC = do_event(0)
    if RC == True:
        t = TestTimer(SECONDS_PER_MINUTE, timer_tick)
        t.start()
        write_log ("TEST STARTED")
        t.join()
        write_log ("thread joined")
    
    if RC == False:
        write_log("TEST FAILED AFTER " + str(ELAPSED_HOUR) +"h")
        abort_pos()
        exit(-1)
    else:
        write_log("TEST SUCCESS")
        if exit_pos() == False:
            abort_pos()
        exit(0)

if __name__ == "__main__":
    write_log("============== START 72 HOUR TEST ==============")
    write_log("SECONDS PER MIN: " + str(SECONDS_PER_MINUTE))
    write_log("MINUTES PER HOUR: " + str(MINUTES_PER_HOUR))
    main(sys.argv[1])
