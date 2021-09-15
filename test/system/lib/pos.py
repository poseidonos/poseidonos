#!/usr/bin/env python3

import subprocess
import sys
import cli
import json_parser

POS_ROOT = '../../../'
LOG_PATH = 'pos.log'
TR_ADDR = "10.100.11.21"
TR_TYPE = 'TCP'

isExecuted = False

def set_addr(addr):
    global TR_ADDR
    TR_ADDR = addr

def start_pos():
    global isExecuted
    if isExecuted == True:
        return

    global pos_proc
    pos_execution = POS_ROOT + "bin/poseidonos"
    with open(LOG_PATH, "w") as output_file:
        pos_proc = subprocess.Popen(pos_execution, \
                stdout=output_file, stderr=output_file)
        isExecuted = True
    subprocess.call(["sleep", "3"])
    pos_bringup = POS_ROOT + "/test/system/lib/bring_up.sh"
    subprocess.call([pos_bringup, "-t", TR_TYPE, "-a", TR_ADDR])

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