#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import ibofos
import cli
import test_result
import ibofos_constant
import MOUNT_VOL_BASIC_1
import CREATE_ARRAY_BASIC_1

DATA = "unvme-ns-0,unvme-ns-1,unvme-ns-2"
SPARE = "unvme-ns-3"

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(detail):
    data = json.loads(detail)
    state = data['Response']['info']['state']
    capacity = data['Response']['info']['capacity']
    used = data['Response']['info']['used']

    if state == "NORMAL" and capacity != 0 and used == 0 :
        return "pass"
    return "fail"

def set_result(detail):
    result = check_result(detail)
    code = json_parser.get_response_code(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_VOL_BASIC_1.execute()
    cli.unmount_ibofos()
    cli.delete_array("")
    cli.create_array("uram0", DATA, SPARE, "", "")
    cli.mount_ibofos()
    out = cli.get_ibofos_info()

    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()