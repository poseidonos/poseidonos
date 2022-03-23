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
import MOUNT_ARRAY_BASIC
import volume

VOL_NAME = "vol1"
VOL_SIZE = pos_constant.SIZE_1GB
VOL_IOPS = 0
VOL_BW = 0

SPARE = MOUNT_ARRAY_BASIC.SPARE
ANY_DATA = MOUNT_ARRAY_BASIC.ANY_DATA
ARRAYNAME = MOUNT_ARRAY_BASIC.ARRAYNAME

def check_result():
    detail = cli.list_volume(ARRAYNAME)
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


def execute():
    MOUNT_ARRAY_BASIC.execute()
    out = cli.create_volume(VOL_NAME, str(VOL_SIZE), "", "", ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result()
    ret = api.set_result_manually(out, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
