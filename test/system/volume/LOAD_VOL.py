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
import ibofos_constant
import volume
import CREATE_VOL_BASIC_1
import CREATE_VOL_BASIC_2
import CREATE_VOL_BASIC_3

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(detail):
    expected_list = []
    expected_list.append(volume.Volume(CREATE_VOL_BASIC_1.VOL_NAME, 
        CREATE_VOL_BASIC_1.VOL_SIZE, CREATE_VOL_BASIC_1.VOL_IOPS, CREATE_VOL_BASIC_1.VOL_BW))
    expected_list.append(volume.Volume(CREATE_VOL_BASIC_2.VOL_NAME, 
        CREATE_VOL_BASIC_2.VOL_SIZE, CREATE_VOL_BASIC_2.VOL_IOPS, CREATE_VOL_BASIC_2.VOL_BW))
    expected_list.append(volume.Volume(CREATE_VOL_BASIC_3.VOL_NAME, 
        CREATE_VOL_BASIC_3.VOL_SIZE, CREATE_VOL_BASIC_3.VOL_IOPS, CREATE_VOL_BASIC_3.VOL_BW))
  
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

def set_result():
    out = cli.list_volume("")
    result = check_result(out)
    code = json_parser.get_response_code(out)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    CREATE_VOL_BASIC_1.execute()
    CREATE_VOL_BASIC_2.execute()
    CREATE_VOL_BASIC_3.execute()
    cli.unmount_ibofos()
    ibofos.exit_ibofos()
    ibofos.start_ibofos()
    cli.scan_device()
    cli.load_array("")
    cli.mount_ibofos()

if __name__ == "__main__":
    execute()
    set_result()
    ibofos.kill_ibofos()