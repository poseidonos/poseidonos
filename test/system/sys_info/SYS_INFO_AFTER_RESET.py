#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import test_result
import pos_constant
import DELETE_ARRAY_WITH_VOL

DATA = "unvme-ns-0,unvme-ns-1,unvme-ns-2"
SPARE = "unvme-ns-3"


def check_result(detail):
    state = json_parser.get_state(detail)
    capacity = json_parser.get_capacity(detail)
    used =json_parser.get_used(detail)

    if state == "NORMAL" and capacity != 0 and used == 0 :
        return "pass"
    return "fail"

def set_result(detail):
    result = check_result(detail)
    code = json_parser.get_response_code(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    DELETE_ARRAY_WITH_VOL.execute()
    cli.create_array("uram0", DATA, SPARE, "", "")
    cli.mount_array(DELETE_ARRAY_WITH_VOL.ARRAYNAME)
    out = cli.get_pos_info()
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()