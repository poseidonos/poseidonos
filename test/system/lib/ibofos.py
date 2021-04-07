#!/usr/bin/env python3

import subprocess
import sys
import cli

IBOFOS_ROOT = '../../../'
LOG_PATH = 'ibofos.log'
TR_ADDR = "10.100.11.21"
TR_TYPE = 'TCP'

isExecuted = False

def set_addr(addr):
    global TR_ADDR
    TR_ADDR = addr

def start_ibofos():
    global isExecuted
    if isExecuted == True:
        return

    global ibof_proc
    ibof_execution = IBOFOS_ROOT + "bin/ibofos"
    with open(LOG_PATH, "w") as output_file:
        ibof_proc = subprocess.Popen(ibof_execution, \
                stdout=output_file, stderr=output_file)
        isExecuted = True
    subprocess.call(["sleep", "3"])
    ibofos_bringup = IBOFOS_ROOT + "/test/system/lib/bring_up.sh"
    subprocess.call([ibofos_bringup, "-t", TR_TYPE, "-a", TR_ADDR])

def exit_ibofos():
    out = cli.exit_ibofos()
    ibof_proc.wait()
    global isExecuted
    isExecuted = False
    return out

def kill_ibofos():
    ibof_proc.kill()
    ibof_proc.wait()
    global isExecuted
    isExecuted = False
    subprocess.call("echo 1 > /sys/bus/pci/rescan",shell=True)
