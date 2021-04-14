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
import DETACH_DEV_DATA
import fio
import time

DETACH_TARGET_DEV = DETACH_DEV_DATA.DETACH_TARGET_DEV
REMAINING_DEV = DETACH_DEV_DATA.REMAINING_DEV
ARRAYNAME = DETACH_DEV_DATA.ARRAYNAME


def check_result():
    out = cli.get_pos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "DEGRADED":
        list = cli.array_info(DETACH_DEV_DATA.ARRAYNAME)
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
    DETACH_DEV_DATA.execute()
    fio_proc = fio.start_fio(0, 30)
    fio.wait_fio(fio_proc)
    out = cli.get_pos_info()
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()