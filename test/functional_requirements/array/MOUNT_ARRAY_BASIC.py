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
import json
import CREATE_ARRAY_BASIC

SPARE = CREATE_ARRAY_BASIC.SPARE
ANY_DATA = CREATE_ARRAY_BASIC.ANY_DATA
ANY_OTHER_DATA = CREATE_ARRAY_BASIC.ANY_OTHER_DATA
ARRAYNAME = CREATE_ARRAY_BASIC.ARRAYNAME


def execute():
    CREATE_ARRAY_BASIC.execute()
    out = cli.mount_array(ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 0, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
