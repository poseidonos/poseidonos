#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")

import json_parser
import pos
import cli
import api
import CREATE_ARRAY_BASIC
import array_device

ARRAYNAME = CREATE_ARRAY_BASIC.ARRAYNAME

def check_result(detail):
    expected_list = []
    expected_list.append(array_device.ArrayDevice("uram0", "BUFFER"))
    expected_list.append(array_device.ArrayDevice("unvme-ns-0", "DATA"))
    expected_list.append(array_device.ArrayDevice("unvme-ns-1", "DATA"))
    expected_list.append(array_device.ArrayDevice("unvme-ns-2", "DATA"))
    expected_list.append(array_device.ArrayDevice("unvme-ns-3", "SPARE"))

    data = json.loads(detail)
    actual_list = []
    for item in data['Response']['result']['data']['devicelist']:
        dev = array_device.ArrayDevice(**item)
        actual_list.append(dev)
    
    for actual in actual_list:
        checked = False
        for expected in expected_list:
            if actual.name == expected.name and actual.type == expected.type:
                checked = True
                break
        if checked == False:
            return "fail"
    return "pass"

def execute():
    CREATE_ARRAY_BASIC.execute()
    out = cli.list_array_device(ARRAYNAME)
    return out

if __name__ == "__main__":
    api.clear_result(__file__)
    out = execute()
    result = check_result(out)
    ret = api.set_result_manually(out, result, __file__)
    pos.kill_pos()
    exit(ret)