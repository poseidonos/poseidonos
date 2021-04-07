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
import fio
import time
import CREATE_ARRAY_NO_SPARE
import DETACH_DEV_DATA_DURING_IO

DETACH_ANOTHER_TARGET_DEV = CREATE_ARRAY_NO_SPARE.DATA_DEV_2

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result():
    out = cli.get_ibofos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "STOP":
        list = cli.array_info("")
        data = json.loads(list)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_ANOTHER_TARGET_DEV :
                return "fail", list
        return "pass", out
    return "fail", out

def set_result(detail):
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
    DETACH_DEV_DATA_DURING_IO.execute()
    fio_proc = fio.start_fio(0, 60)
    time.sleep(1)
    ibofos_util.pci_detach(DETACH_ANOTHER_TARGET_DEV)
    time.sleep(1)
    out = cli.get_ibofos_info()
    fio.wait_fio(fio_proc)
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()
    ibofos_util.pci_rescan()