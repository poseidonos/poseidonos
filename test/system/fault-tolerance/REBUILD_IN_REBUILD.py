#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import pos
import pos_util
import cli
import test_result
import json
import MOUNT_VOL_BASIC_1
import fio
import time
DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.ANY_DATA

def check_result():
    out = cli.get_pos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "NORMAL":
        return "pass", out
    return "fail", out

def set_result():
    detail = cli.get_pos_info()
    code = json_parser.get_response_code(detail)
    if code == 0:
        result, out = check_result()
    else:
        result = "fail"
        out = detail

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 200)
    time.sleep(10)
    pos_util.pci_detach_and_attach(DETACH_TARGET_DEV)
    detach_dev_timeout = 80
    time.sleep(detach_dev_timeout)
    print(cli.list_device())
    spare_dev_newly_attached = "unvme-ns-4"
    result = cli.add_device(spare_dev_newly_attached, MOUNT_VOL_BASIC_1.ARRAYNAME)
    code = json_parser.get_response_code(result)
    if code == 0:
        print ("device added successfully")
        rebuild_trigger_timeout = 100
        rebuild_started = False
        for i in range(rebuild_trigger_timeout):
            out = cli.get_pos_info()
            if out.find("REBUILD") >= 0:
                print ("rebuilding started")
                rebuild_duration = 5
                time.sleep(rebuild_duration)
                rebuild_started = True
                break
            time.sleep(1)
        
        if rebuild_started == False:
            fio.stop_fio(fio_proc)
            return False

        pos_util.pci_detach_and_attach(MOUNT_VOL_BASIC_1.SPARE)
        #waiting for rebuild stopped
        print ("Waiting for rebuild stopped")
        rebuild_stop_delay = 60
        time.sleep(rebuild_stop_delay)
        
        rebuild_started = False
        while True:
            out = cli.get_pos_info()
            if out.find("REBUILD") == -1 and rebuild_started == True:
                print ("2nd rebuilding done")
                fio.wait_fio(fio_proc)
                return True
            elif rebuild_started == False and out.find("REBUILD") >= 0:
                print ("2nd rebuilding started")
                rebuild_started = True
            time.sleep(1)
    else:
        print ("device added failure")
        fio.stop_fio(fio_proc)
        return False

if __name__ == "__main__":
    test_result.clear_result(__file__)
    ret = execute()
    if ret == False:
        with open(__file__ + ".result", "w") as result_file:
            result_file.write("fail")
    else :
        set_result()

    pos.kill_pos()
    pos_util.pci_rescan()