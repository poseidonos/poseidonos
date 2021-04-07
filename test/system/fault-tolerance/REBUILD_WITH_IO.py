#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import ibofos
import ibofos_util
import cli
import test_result
import json
import MOUNT_VOL_BASIC_1
import fio
import time
DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.ANY_DATA

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result():
    out = cli.get_ibofos_info()
    data = json.loads(out)
    if data['Response']['info']['situation'] == "NORMAL":
        list = cli.array_info("")
        data = json.loads(list)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_TARGET_DEV :
                return "fail", list
        return "pass", out
    return "fail", out

def set_result():
    detail = cli.get_ibofos_info()
    code = json_parser.get_response_code(detail)
    if code == 0:
        result, out = check_result()
    else:
        result = "fail"
        out = detail

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 20)
    fio.wait_fio(fio_proc)
    ibofos_util.pci_detach(DETACH_TARGET_DEV)
    time.sleep(0.1)
    
    fio_proc = fio.start_fio(0, 20)
    fio.wait_fio(fio_proc)

    rebuild_started = False
    wait_threshold = 60
    for i in range(0, wait_threshold):
        out = cli.get_ibofos_info()
        if out.find("REBUILDING") >= 0:
            print ("rebuilding started")
            rebuild_started = True
            break
        time.sleep(1)
    
    if rebuild_started == True: 
        fio_proc2 = fio.start_fio(0, 120)
        while True:
            out = cli.get_ibofos_info()
            if out.find("REBUILDING") == -1:
                print ("rebuilding done")
                fio.wait_fio(fio_proc2)
                break

if __name__ == "__main__":
    execute()
    set_result()
    ibofos.kill_ibofos()
    ibofos_util.pci_rescan()