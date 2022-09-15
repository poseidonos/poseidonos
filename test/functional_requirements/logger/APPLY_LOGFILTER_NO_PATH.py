#!/usr/bin/env python3
import subprocess
import os
import sys

sys.path.append("../")
sys.path.append("../../system/lib/")
sys.path.append("../system_overall/")

import json_parser
import pos
import cli
import api
import json
import START_POS_BASIC


def execute():
    START_POS_BASIC.execute()
    out = cli.apply_log_filter()
    print (out)
    return out


if __name__ == "__main__":
    if len(sys.argv) >= 2:
        pos.set_addr(sys.argv[1])
    api.clear_result(__file__)
    out = execute()
    ret = api.set_result_by_code_eq(out, 4581, __file__)
    pos.flush_and_kill_pos()
    exit(ret)
