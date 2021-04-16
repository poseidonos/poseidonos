#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")

import json_parser
import pos
import cli
import test_result
import json
import MOUNT_VOL_BASIC_1
import fio
import time

ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME

def check_result():
    out = cli.array_info(ARRAYNAME)
    if json_parser.is_online(out) == False:
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    if result == "pass":
        result = check_result()
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    MOUNT_VOL_BASIC_1.execute()
    fio_proc = fio.start_fio(0, 60)
    time.sleep(5)
    out = cli.unmount_array(ARRAYNAME)
    fio.stop_fio(fio_proc)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()