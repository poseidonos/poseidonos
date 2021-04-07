#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import ibofos
import cli
import test_result
import MOUNT_VOL_BASIC_1
VOL_NAME = MOUNT_VOL_BASIC_1.VOL_NAME

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result(expected):
    detail = cli.get_ibofos_info()
    data = json.loads(detail)
    used = data['Response']['result']['data']['used']
    result = "fail"
    if used != 0 and used == expected:
        result = "pass"
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (0)" + "\n" + detail)

def execute():
    clear_result()
    MOUNT_VOL_BASIC_1.execute()
    out = cli.get_ibofos_info()
    print(out)
    data = json.loads(out)
    used = data['Response']['result']['data']['used']
    cli.unmount_ibofos()
    cli.mount_ibofos()
    return used

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()