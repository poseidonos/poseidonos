#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import ibofos
import cli
import test_result
import json
import time
import MOUNT_ARRAY_DEGRADED_1

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result():
    out = cli.get_ibofos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "BUSY":
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    if code == 0:
        result = check_result()
    else:
        result = "fail"

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    clear_result()
    out = MOUNT_ARRAY_DEGRADED_1.execute()
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()