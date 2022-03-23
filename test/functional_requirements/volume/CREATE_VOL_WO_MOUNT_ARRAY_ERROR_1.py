#!/usr/bin/env python3
import subprocess
import os
import sys
import json
sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../array/")

import json_parser
import pos
import cli
import api
import pos_constant
import CREATE_ARRAY_BASIC
ARRAYNAME = CREATE_ARRAY_BASIC.ARRAYNAME


def execute():
    CREATE_ARRAY_BASIC.execute()
    size = pos_constant.SIZE_1GB
    name = "vol1"
    out = cli.create_volume(name , str(size),"","",ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_ne(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)