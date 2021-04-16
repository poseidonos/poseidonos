#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import pos_constant
import json_parser
import pos
import pos_util
import cli
import test_result
import json
import fio
import time
import DETACH_DEV_DATA_DURING_IO

DETACH_TARGET = DETACH_DEV_DATA_DURING_IO.ANY_OTHER_DATA
ARRAYNAME = DETACH_DEV_DATA_DURING_IO.ARRAYNAME

def check_result():
    out = cli.array_info(ARRAYNAME)
    situ = json_parser.get_situation(out)
    if situ == "FAULT":
        return "pass", out
    return "fail", out

def set_result():
    result, out = check_result()
    code = json_parser.get_response_code(out)

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    DETACH_DEV_DATA_DURING_IO.execute()
    pos_util.pci_detach(DETACH_TARGET)

if __name__ == "__main__":
    test_result.clear_result(__file__)
    execute()
    set_result()
    pos.kill_pos()
    pos_util.pci_rescan()