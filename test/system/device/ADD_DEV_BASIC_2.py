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
import ADD_DEV_BASIC_1

SPARE_DEV = ADD_DEV_BASIC_1.SPARE_DEV

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(out):
    data = json.loads(out)
    list = []
    for item in data['Response']['result']['data']['devicelist']:
        if item['type'] == "SPARE" and item['name'] == SPARE_DEV :
            return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    if code == 0:
        result = check_result(detail)
    else:
        result = "fail"

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    clear_result()
    ADD_DEV_BASIC_1.execute()
    cli.unmount_ibofos()
    ibofos.exit_ibofos()
    ibofos.start_ibofos()
    cli.scan_device()
    cli.load_array("")
    cli.mount_ibofos()
    out = cli.array_info("")
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()