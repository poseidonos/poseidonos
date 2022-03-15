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
import pos_util

def execute(isSingleArray = True):
    pos_util.kill_process("poseidonos")
    pos_util.pci_rescan()
    if isSingleArray is True:
        return pos.start_pos()
    else:
        return pos.start_pos_for_two_arrays()

if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    res = execute()
    ret = -1
    if res is True:
        out = cli.get_pos_info()
        ret = api.set_result_by_code_eq(out, 0, __file__)
        pos.flush_and_kill_pos()
    exit(ret)