#!/usr/bin/env python3
import subprocess
import sys
import cli
import json_parser
import time
import os
sys.path.append("../../functional_requirements/")
import api

POS_ROOT = '../../../'
LOG_PATH = 'pos.log'
TR_ADDR = "10.100.11.21"
TR_TYPE = 'TCP'
WT_FILE = os.path.dirname(os.path.realpath(__file__)) + '/WT_ENABLED'

isExecuted = False

def set_addr(addr):
    global TR_ADDR
    TR_ADDR = addr


def start_pos_without_bringup():
    global isExecuted
    print("start pos")
    if isExecuted == True:
        print("pos is already runnning")
        return

    global pos_proc
    pos_execution = POS_ROOT + "bin/poseidonos"
    with open(LOG_PATH, "w") as output_file:
        pos_proc = subprocess.Popen(pos_execution, \
                stdout=output_file, stderr=output_file)
        isExecuted = True
    subprocess.call(["sleep", "3"])
    tryCnt = 0
    maxRetry = 20
    ret = api.check_pos_alive()
    while ret is False:
        tryCnt += 1
        if tryCnt == maxRetry:
            print("failed to initialize pos server")
            kill_pos()
            return False
        time.sleep(1)
        print("waiting for pos server initialization, " + str(tryCnt) + " time(s) retried")
        ret = api.check_pos_alive()
    print("pos is ready")
    return True


def start_pos():
    ret = start_pos_without_bringup()
    if ret is False:
        return False
    pos_bringup = POS_ROOT + "/test/system/lib/bring_up.sh"
    subprocess.call([pos_bringup, "-t", TR_TYPE, "-a", TR_ADDR])

    time.sleep(10)
    cli.start_telemetry()

    return True

def start_pos_for_two_arrays():
    ret = start_pos_without_bringup()
    if ret is False:
        return False
    pos_bringup = POS_ROOT + "/test/system/lib/bring_up_for_two_arrays.sh"
    subprocess.call([pos_bringup, "-t", TR_TYPE, "-a", TR_ADDR])
    return True


def exit_pos():
    out = cli.exit_pos()
    code = json_parser.get_response_code(out)
    if code == 0:
        pos_proc.wait()
        global isExecuted
        isExecuted = False
    return out

def mbr_reset():
    global isExecuted
    if isExecuted == True:
        return

    pos_mbr_reset = POS_ROOT + "/test/script/mbr_reset.sh"
    subprocess.call([pos_mbr_reset])

def kill_pos():
    pos_proc.kill()
    pos_proc.wait()
    global isExecuted
    isExecuted = False
    subprocess.call("echo 1 > /sys/bus/pci/rescan",shell=True)

def flush_and_kill_pos():
    cli.wbt_request("flush_gcov","")
    kill_pos()

def enable_wt():
    print ("trying to enable write through: " + WT_FILE)
    if os.path.exists(WT_FILE) is False:
        with open(WT_FILE, "w") as flag:
            flag.write("WT is enabled")
    if is_wt_enabled() is True:
        print ("write through enabled")


def disable_wt():
    print ("write through disabled: " + WT_FILE)
    if os.path.exists(WT_FILE):
        os.remove(WT_FILE)


def is_wt_enabled():
    ret = os.path.exists(WT_FILE)
    return ret
