#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../lib/")
sys.path.append("../volume/")

import json_parser
import pos
import cli
import test_result
import pos_constant
import CREATE_VOL_BASIC_1


def set_result():
    out = cli.array_info(ARRAYNAME)
    code = json_parser.get_response_code(out)
    result = "fail"
    if code == 0:
        state = json_parser.get_state(out)
        capacity = json_parser.get_capacity(out)
        used =json_parser.get_used(out)
        if state == "NORMAL" and capacity != 0 and used != 0 and used <= capacity:
            result = "pass"
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    CREATE_VOL_BASIC_1.execute()

if __name__ == "__main__":
    test_result.clear_result(__file__)
    execute()
    set_result()
    pos.kill_pos()