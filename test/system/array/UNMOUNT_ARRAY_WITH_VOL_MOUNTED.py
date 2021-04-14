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
import MOUNT_VOL_BASIC_1

ARRAYNAME = MOUNT_VOL_BASIC_1.ARRAYNAME

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    MOUNT_VOL_BASIC_1.execute()
    out = cli.unmount_array(MOUNT_VOL_BASIC_1.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()