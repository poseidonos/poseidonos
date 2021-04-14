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
import MOUNT_VOL_NO_SPARE
import fio
import time
import CREATE_ARRAY_NO_SPARE
DETACH_TARGET_DEV = CREATE_ARRAY_NO_SPARE.DATA_DEV_1
ANY_OTHER_DATA = CREATE_ARRAY_NO_SPARE.DATA_DEV_2


def check_result():
    out = cli.get_pos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "DEGRADED":
        list = cli.array_info("")
        data = json.loads(list)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_TARGET_DEV :
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
    MOUNT_VOL_NO_SPARE.execute()
    fio_proc = fio.start_fio(0, 30)
    time.sleep(1)
    pos_util.pci_detach(DETACH_TARGET_DEV)
    time.sleep(1)
    out = cli.get_pos_info()
    return out, fio_proc

if __name__ == "__main__":
    out, fio_proc = execute()
    fio.wait_fio(fio_proc)
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()