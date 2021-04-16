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
ARRAYNAME = MOUNT_VOL_NO_SPARE.ARRAYNAME

def check_result():
    out = cli.array_info(ARRAYNAME)
    situ = json_parser.get_situation(out)
    if situ == "DEGRADED":
        data = json.loads(out)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_TARGET_DEV :
                return "fail", out
        return "pass", out
    return "fail", out

def set_result():
    result, out = check_result()
    code = json_parser.get_response_code(out)

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_VOL_NO_SPARE.execute()
    fio_proc = fio.start_fio(0, 30)
    time.sleep(1)
    pos_util.pci_detach(DETACH_TARGET_DEV)
    time.sleep(1)
    return fio_proc

if __name__ == "__main__":
    fio_proc = execute()
    fio.wait_fio(fio_proc)
    set_result()
    pos.kill_pos()
    pos_util.pci_rescan()