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
import DELETE_ARRAY_WITH_VOL

ARRAYNAME = DELETE_ARRAY_WITH_VOL.ARRAYNAME
DATA = "unvme-ns-0,unvme-ns-1,unvme-ns-2"
SPARE = "unvme-ns-3"


def check_result():
    if api.get_used_size(ARRAYNAME) == 0:
        return "pass"
    return "fail"


def execute():
    DELETE_ARRAY_WITH_VOL.execute()
    cli.create_array("uram0", DATA, SPARE, ARRAYNAME, "")
    cli.mount_array(ARRAYNAME)
    out = cli.array_info(ARRAYNAME)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result()
    ret = api.set_result_manually(out, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
