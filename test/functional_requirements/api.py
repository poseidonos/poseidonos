#!/usr/bin/env python3
import subprocess
import os
import sys
import time
import json

sys.path.append("../system/lib/")
import json_parser
import pos
import cli
import pos_util

def clear_result(file):
    resultFile = file + ".result"
    if os.path.exists(resultFile):
        os.remove(resultFile)

def write_result(outputfile, result, code, message):
    with open(outputfile + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + message)

def result_to_int(result):
    if result == "pass":
        return 0
    return -1

def set_result_by_code_eq(response, code_eq, outputfile):
    code = json_parser.get_response_code(response)
    result = "pass"
    if code != code_eq:
        result = "fail"
    write_result(outputfile, result, code, response)
    return result_to_int(result)

def set_result_by_code_ne(response, code_ne, outputfile):
    code = json_parser.get_response_code(response)
    result = "pass"
    if code == code_ne:
        result = "fail"
    write_result(outputfile, result, code, response)
    return result_to_int(result)

def set_result_by_state_eq(arrayname, response, state_eq, outputfile):
    code = json_parser.get_response_code(response)
    result = "pass"
    if check_state(arrayname, state_eq) == False:
        result = "fail"
    write_result(outputfile, result, code, response)
    return result_to_int(result)

def set_result_by_state_ne(arrayname, response, state_ne, outputfile):
    code = json_parser.get_response_code(response)
    result = "pass"
    if check_state(arrayname, state_ne) == True:
        result = "fail"
    write_result(outputfile, result, code, response)
    return result_to_int(result)

def set_result_by_situation_eq(arrayname, response, situation_eq, outputfile):
    code = json_parser.get_response_code(response)
    result = "pass"
    if check_situation(arrayname, situation_eq) == False:
        result = "fail"
    write_result(outputfile, result, code, response)
    return result_to_int(result)

def set_result_manually(response, result, outputfile):
    code = json_parser.get_response_code(response)
    write_result(outputfile, result, code, response)
    return result_to_int(result)

def check_pos_alive():
    response = cli.get_pos_info()
    try:
        data = json.loads(response)
    except ValueError as e:
        return False
    return True

def get_state(arrayname):
    response = cli.array_info(arrayname)
    return json_parser.get_state(response)

def get_situation(arrayname):
    response = cli.array_info(arrayname)
    return json_parser.get_situation(response)

def check_state(arrayname, state_expected):
    state = get_state(arrayname)
    if state == state_expected:
        return True
    return False

def check_situation(arrayname, situation_expected):
    situation = get_situation(arrayname)
    if situation == situation_expected:
        return True
    return False

def wait_state(arrayname, state_expected, timeout_ms = -1):
    elapsed_ms = 0
    while check_state(arrayname, state_expected) == False:
        time.sleep(0.01)
        elapsed_ms += 10
        if elapsed_ms % 1000 is 0:
            print (get_situation(arrayname))
        if timeout_ms != -1 and elapsed_ms > timeout_ms:
            return False
    return True

def wait_situation(arrayname, situation_expected, timeout_ms = -1):
    elapsed_ms = 0
    while check_situation(arrayname, situation_expected) == False:
        time.sleep(0.01)
        elapsed_ms += 10
        if elapsed_ms % 1000 is 0:
            print (get_situation(arrayname))
        if timeout_ms != -1 and elapsed_ms > timeout_ms:
            return False
    return True


def wait_situation_changed(arrayname, curr_situation, timeout_ms = -1):
    elapsed_ms = 0
    situ = get_situation(arrayname)
    while situ == curr_situation:
        time.sleep(0.01)
        elapsed_ms += 10
        if elapsed_ms % 1000 is 0:
            print (situ)
        if timeout_ms != -1 and elapsed_ms > timeout_ms:
            return situ
        situ = get_situation(arrayname)
    return situ


def is_offline(arrayname):
    return get_state(arrayname) == "OFFLINE"

def is_online(arrayname):
    state = get_state(arrayname)
    if state == "NORMAL" or state == "BUSY":
        return True
    return False

def is_broken(arrayname):
    return get_situation(arrayname) == "FAULT"

def is_mounting(arrayname):
    return get_situation(arrayname) == "TRY_MOUNT"

def is_unmounting(arrayname):
    return get_situation(arrayname) == "TRY_UNMOUNT"

def is_recovering(arrayname):
    return get_situation(arrayname) == "JOURNAL_RECOVERY"

def is_rebuilding(arrayname):
    return get_situation(arrayname) == "REBUILDING"

def is_degraded(arrayname):
    return get_situation(arrayname) == "DEGRADED"

def is_array_deletable(arrayname):
    return is_offline(arrayname) or is_broken(arrayname)

def is_data_device(arrayname, devicename):
    response = cli.array_info(arrayname)
    return json_parser.is_data_device(response)

def is_spare_device(arrayname, devicename):
    response = cli.array_info(arrayname)
    return json_parser.is_spare_device(response, devicename)

def is_buffer_device(arrayname, devicename):
    response = cli.array_info(arrayname)
    return json_parser.is_buffer_device(response, devicename)

def is_device_in_the_array(arrayname, devicename):
    response = cli.array_info(arrayname)
    return json_parser.is_array_device(response, devicename)

def is_system_device(devicename):
    response = cli.list_device()
    return json_parser.is_system_device(response, devicename)

def is_device_exists(devicename):
    response = cli.list_device()
    return json_parser.is_device_exists(response, devicename)

def is_array_device(devicename):
    return is_system_device(devicename) == False and is_device_exists(devicename) == True

def is_ssd(devicename):
    response = cli.list_device()
    return json_parser.is_ssd(response, devicename)

def detach_ssd(devicename):
    pos_util.pci_detach(devicename)
    time.sleep(0.1)

def detach_ssd_and_attach(devicename):
    pos_util.pci_detach_and_attach(devicename)
    time.sleep(0.1)

def rescan_ssd():
    pos_util.pci_rescan()
    time.sleep(0.1)

def get_used_size(arrayname):
    arrayinfo = cli.array_info(arrayname)
    print (arrayinfo)
    data = json.loads(arrayinfo)
    return data['Response']['result']['data']['used']


def set_rebuild_autostart(val):
    return cli.update_config("rebuild", "auto_start", val, "bool")