#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import pos
import pos_util
import cli
import api
import json
import MOUNT_VOL_BASIC_1
import fio
import time
DETACH_TARGET_DEV = MOUNT_VOL_BASIC_1.ANY_DATA
ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME

def check_result():
    out = cli.array_info(ARRAYNAME)
    state = json_parser.get_state(out)
    if state == "NORMAL":
        data = json.loads(out)
        for item in data['Response']['result']['data']['devicelist']:
            if item['name'] == DETACH_TARGET_DEV :
                return "fail", out
        return "pass", out
    return "fail", out

def set_result():
    result, out = check_result()

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 20)
    fio.wait_fio(fio_proc)
    api.detach_ssd(DETACH_TARGET_DEV)
    
    if api.wait_situation(ARRAYNAME, "REBUILDING") == True:
        fio_proc2 = fio.start_fio(0, 120)
        if api.wait_situation(ARRAYNAME, "NORMAL") == True:
                fio.stop_fio(fio_proc2)
                return "pass"
        fio.stop_fio(fio_proc2)
    return "fail"

if __name__ == "__main__":
    api.clear_result(__file__)
    result = execute()
    ret = api.set_result_manually(cli.array_info(ARRAYNAME), result, __file__)
    pos.kill_pos()
    exit(ret)