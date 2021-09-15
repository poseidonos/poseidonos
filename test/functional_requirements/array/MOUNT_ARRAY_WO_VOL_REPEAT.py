#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")

import json_parser
import pos
import cli
import api
import CREATE_ARRAY_BASIC

ARRAYNAME = CREATE_ARRAY_BASIC.ARRAYNAME


def execute():
    out = CREATE_ARRAY_BASIC.execute()
    code = 0
    repeat = 30
    for i in range(0, repeat):
        print("TEST(" + str(i + 1) + ")")
        out = cli.mount_array(ARRAYNAME)
        print("mount: " + out)
        code = json_parser.get_response_code(out)
        if code != 0:
            return out
        out = cli.unmount_array(ARRAYNAME)
        print("unmount: " + out)
        code = json_parser.get_response_code(out)
        if code != 0:
            return out
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
    