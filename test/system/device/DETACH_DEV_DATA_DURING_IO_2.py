#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import ibofos_constant
import json_parser
import ibofos
import ibofos_util
import cli
import test_result
import json
import fio
import time
import MOUNT_ARRAY_BASIC_1

SIZE_20GB = ibofos_constant.SIZE_1GB * 20
DETACH_TARGET_DEV_1 = MOUNT_ARRAY_BASIC_1.ANY_DATA
DETACH_TARGET_DEV_2 = MOUNT_ARRAY_BASIC_1.ANY_OTHER_DATA

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result():
    out = cli.get_ibofos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "STOP":
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    out = detail
    if code == 0:
        result = check_result()
    else:
        result = "fail"

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_ARRAY_BASIC_1.execute()
    cli.create_volume("vol1", str(SIZE_20GB), "", "", "")
    cli.create_volume("vol2", str(SIZE_20GB), "", "", "")
    cli.create_volume("vol3", str(SIZE_20GB), "", "", "")
    cli.create_volume("vol4", str(SIZE_20GB), "", "", "")
    cli.create_volume("vol5", str(SIZE_20GB), "", "", "")
    cli.mount_volume("vol1", "", "")
    cli.mount_volume("vol2", "", "")
    cli.mount_volume("vol3", "", "")
    cli.mount_volume("vol4", "", "")
    cli.mount_volume("vol5", "", "")
    print ("five volumes are mounted")
    
    time.sleep(1)
    fio_proc = fio.start_fio(0, 20)
    time.sleep(5)

    ibofos_util.pci_detach(DETACH_TARGET_DEV_1)
    print ("first device has been detached")
    time.sleep(1)

    ibofos_util.pci_detach(DETACH_TARGET_DEV_2)
    print ("second device has been detached")
    time.sleep(1)

    out = cli.get_ibofos_info()
    print ("info : " + out)
    fio.stop_fio(fio_proc)
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()
    ibofos_util.pci_rescan()