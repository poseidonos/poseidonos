#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import pos
import cli
import test_result
import json
import time
import pos_util
import CREATE_ARRAY_BASIC

def check_result(out):
    data = json.loads(out)
    for item in data['Response']['result']['data']['devicelist']:
        if item['type'] == "SPARE" and item['name'] != "":
            return "fail"
    return "pass"

def set_result(detail):
    out = detail
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    if result == "pass":
        out = cli.list_array_device(CREATE_ARRAY_BASIC.ARRAYNAME)
        result = check_result(out)
    
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    CREATE_ARRAY_BASIC.execute()
    spare = CREATE_ARRAY_BASIC.SPARE
    pos_util.pci_detach(spare)
    time.sleep(0.1)
    cli.unmount_array(CREATE_ARRAY_BASIC.ARRAYNAME)
    pos.exit_pos()
    time.sleep(5)
    pos.start_pos()
    cli.scan_device()
    out = cli.mount_array(CREATE_ARRAY_BASIC.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()