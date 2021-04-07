#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../exit/")

import json_parser
import ibofos
import cli
import test_result
import json
import EXIT_BASIC_1

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(detail):
    data = json.loads(detail)
    state = data['Response']['info']['state']
    if state == "OFFLINE":
        return "pass"
    return "fail"

def set_result():
    out = cli.get_ibofos_info()
    code = json_parser.get_response_code(out)
    result = check_result(out)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    clear_result()
    EXIT_BASIC_1.execute()
    ibofos.start_ibofos()
    cli.scan_device()

if __name__ == "__main__":
    execute()
    set_result()
    ibofos.kill_ibofos()