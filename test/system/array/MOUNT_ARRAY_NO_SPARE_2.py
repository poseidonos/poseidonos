#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import ibofos
import cli
import test_result
import json
import time
import ibofos_util
import MOUNT_ARRAY_BASIC_1

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(out):
    data = json.loads(out)
    for item in data['Response']['result']['data']['devicelist']:
        if item['type'] == "SPARE" and item['name'] == MOUNT_ARRAY_BASIC_1.SPARE:
            return "fail"
    return "pass"

def set_result(detail):
    out = detail
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    if result == "pass":
        out = cli.array_info("")
        result = check_result(out)
    
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_ARRAY_BASIC_1.execute()
    spare = MOUNT_ARRAY_BASIC_1.SPARE
    ibofos_util.pci_detach(spare)
    time.sleep(0.1)
    cli.unmount_ibofos()
    ibofos.exit_ibofos()
    ibofos.start_ibofos()
    cli.scan_device()
    cli.load_array("")
    out = cli.mount_ibofos()
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    ibofos.kill_ibofos()
    ibofos_util.pci_rescan()