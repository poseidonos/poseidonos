#!/usr/bin/env python3
import subprocess
import os
import sys
import json
import json_parser
import pos
import cli
import api
import test_result
import CREATE_VOL_BASIC_1
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../volume/")


def clear_result():
    if os.path.exists(__file__ + ".result"):
        os.remove(__file__ + ".result")


def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = test_result.expect_true(code)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)


def execute():
    clear_result()
    CREATE_VOL_BASIC_1.execute()
    subnqn = "nqn.2019-04.pos:subsystem1"
    out = cli.delete_subsystem(subnqn)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    out = execute()
    set_result(out)
    pos.flush_and_kill_pos()
