#!/usr/bin/env python3
import subprocess
import time
import os
import sys
sys.path.append("../")
sys.path.append("../../system/lib/")

import json_parser
import pos
import cli
import pos_util
import api
import CREATE_ARRAY_NO_SPARE

ARRAYNAME = CREATE_ARRAY_NO_SPARE.ARRAYNAME


def check_result(detail):
    code = json_parser.get_response_code(detail)
    if detail.find("\"class\": \"ARRAY\"") is -1:
        return "pass"
    return "fail"


def execute():
    out = CREATE_ARRAY_NO_SPARE.execute()
    time.sleep(1)
    pos_util.pci_detach_and_attach(CREATE_ARRAY_NO_SPARE.DATA_DEV_1)
    time.sleep(30)
    out = cli.list_device()
    cli.delete_array(ARRAYNAME)
    out = cli.list_device()
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    result = check_result(out)
    ret = api.set_result_manually(out, result, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
