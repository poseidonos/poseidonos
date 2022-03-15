#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import pos_constant
import MOUNT_RAID_NONE_ARRAY
import volume

VOL_NAME = "vol1"
VOL_SIZE = pos_constant.SIZE_1GB
VOL_IOPS = 0
VOL_BW = 0

ARRAYNAME = MOUNT_RAID_NONE_ARRAY.ARRAYNAME

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(detail):
    expected_list = []
    expected_list.append(volume.Volume(VOL_NAME, VOL_SIZE, VOL_IOPS, VOL_BW))

    data = json.loads(detail)
    actual_list = []
    for item in data['Response']['result']['data']['volumes']:
        vol = volume.Volume(item['name'], item['total'], item['maxiops'], item['maxbw'])
        actual_list.append(vol)

    if len(actual_list) != len(expected_list):
        return "fail"
    
    for actual in actual_list:
        checked = False
        for expected in expected_list:
            if actual.name == expected.name and actual.total == expected.total and actual.maxiops == expected.maxiops and actual.maxbw == expected.maxbw:
                checked = True
                break
        if checked == False:
            return "fail"
    return "pass"

def set_result(detail):
    out = cli.list_volume(ARRAYNAME)
    result = check_result(out)
    code = json_parser.get_response_code(out)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    MOUNT_RAID_NONE_ARRAY.execute()
    out = cli.create_volume(VOL_NAME, str(VOL_SIZE), "", "", ARRAYNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    out = execute()
    set_result(out)
    pos.flush_and_kill_pos()