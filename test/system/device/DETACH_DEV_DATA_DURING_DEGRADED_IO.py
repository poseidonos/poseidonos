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


def check_result():
    out = cli.get_pos_info()
    data = json.loads(out)
    if data['Response']['info']['state'] == "BROKEN":
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    out = detail
    if code == 0:
        result = check_result()
    else:
        result = "fail"

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    DETACH_DEV_DATA_DURING_IO.execute()
    pos_util.pci_detach(DETACH_TARGET)

    out = cli.get_pos_info()
    print ("info : " + out)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()