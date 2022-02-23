#!/usr/bin/env python3
'''

Author: SRM
'''

import subprocess
import os
import sys
import json

sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../system_overall/")

import json_parser
import cli
import api
import pos
import pos_util

total_numa = 0
TEST_DEVICE_PREFIX = "URAM_TEST"


def create_device_per_numa():
    global total_numa

    total_numa = os.popen("numactl -H | grep available | awk '{print $2}'").read()
    for numa in range(int(total_numa)):
        name = TEST_DEVICE_PREFIX + str(numa)
        cli.create_uram(name, "4096", "256", numa)
    cli.scan_device()


def get_device_names():
    response = cli.list_device()
    device_list_json = json.loads(response)
    device_list = device_list_json['Response']['result']['data']['devicelist']
    device_names = list()
    for device in device_list:
        device_names.append(device['name'])

    return device_names, response


def find_created_device_in_list(device_names):

    for numa in range(int(total_numa)):
        name = TEST_DEVICE_PREFIX + str(numa)
        if device_names.count(name) != 1:
            return "fail"

    return "pass"


def execute():
    pos_util.kill_process("poseidonos")
    ret = pos.start_pos_without_bringup()
    if ret is False:
        return "fail", ""
    create_device_per_numa()
    device_names, response = get_device_names()
    result = find_created_device_in_list(device_names)

    return result, response


if __name__ == "__main__":
    api.clear_result(__file__)
    result, response = execute()
    ret = api.set_result_manually(response, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)

