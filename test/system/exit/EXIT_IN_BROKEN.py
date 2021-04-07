#!/usr/bin/env python3
import subprocess
import os
import sys
import time
sys.path.append("../lib/")
sys.path.append("../volume/")

import json_parser
import ibofos
import cli
import test_result
import json
import ibofos_util
import MOUNT_VOL_NO_SPARE

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def check_result(detail):
    data = json.loads(detail)
    state = data['Response']['info']['state']
    if state == "OFFLINE":
        return "pass"
    return "fail"

def set_result(detail):
    code = json_parser.get_response_code(detail)
    result = check_result(detail)
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + detail)

def execute():
    clear_result()
    ibofos_util.pci_rescan()
    MOUNT_VOL_NO_SPARE.execute()
    ibofos_util.pci_detach("unvme-ns-0")
    print ("unvme-ns-0 has been detached")
    time.sleep(3)
    ibofos_util.pci_detach("unvme-ns-1")
    print ("unvme-ns-1 has been detached")
    time.sleep(3)
    cli.unmount_ibofos()
    out = ibofos.exit_ibofos()
    return out

if __name__ == "__main__":
    out = execute()
    set_result(out)
    #ibofos.kill_ibofos()