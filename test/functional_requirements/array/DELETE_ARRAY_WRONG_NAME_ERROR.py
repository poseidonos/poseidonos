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

WRONGNAME = CREATE_ARRAY_BASIC.ARRAYNAME + "1"

def execute():
    out = CREATE_ARRAY_BASIC.execute()
    ret = json_parser.get_response_code(out)
    if ret == 0:
        out = cli.delete_array(WRONGNAME)
    return out

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)