#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../exit/")

import json_parser
import pos
import cli
import time
import test_result
import EXIT_POS_AFTER_UNMOUNT_VOL

IBOFOS_ROOT = '../../../'

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    EXIT_POS_AFTER_UNMOUNT_VOL.execute()
    time.sleep(5)
    ibofos_mbr_reset = IBOFOS_ROOT + "/test/script/mbr_reset.sh"
    subprocess.call([ibofos_mbr_reset])
    pos.start_pos()
    cli.scan_device()
    out = cli.array_info(EXIT_POS_AFTER_UNMOUNT_VOL.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
