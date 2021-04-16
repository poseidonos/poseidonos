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
SECOND_DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.SPARE
REMAINING_DEV = "unvme-ns-4"
ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME

def check_result():
    out = cli.array_info(ARRAYNAME)
    situ = json_parser.get_situation(out)
    if situ == "DEGRADED":
        return "pass", out
    return "fail", out

def set_result():
    result, out = check_result()

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 60)
    fio.wait_fio(fio_proc)
    pos_util.pci_detach(DETACH_TARGET_DEV)
    time.sleep(0.1)

    while True:
        out = cli.array_info(ARRAYNAME)
        situ = json_parser.get_situation(out)
        if situ.find("REBUILD") >= 0:
            print ("rebuilding started")
            pos_util.pci_detach_and_attach(SECOND_DETACH_TARGET_DEV)
            break
        time.sleep(1)

    timeout = 80
    for i in range(timeout):
        out = cli.array_info(ARRAYNAME)
        situ = json_parser.get_situation(out)
        if situ.find("REBUILD") < 0:
            break
        time.sleep(1)

if __name__ == "__main__":
    test_result.clear_result(__file__)
    execute()
    set_result()
    pos.kill_pos()
    pos_util.pci_rescan()