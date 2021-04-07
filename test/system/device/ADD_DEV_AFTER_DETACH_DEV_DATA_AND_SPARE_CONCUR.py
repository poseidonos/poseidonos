#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")
sys.path.append("../volume/")
sys.path.append("../array/")

import json_parser
import ibofos
import ibofos_util
import cli
import test_result
import json
import time
import MOUNT_VOL_BASIC_1


DETACH_TARGET_DATA = MOUNT_VOL_BASIC_1.ANY_DATA
DETACH_TARGET_SPARE = MOUNT_VOL_BASIC_1.SPARE
NEW_SPARE = "unvme-ns-4"

def clear_result():
    if os.path.exists( __file__ + ".result"):
        os.remove( __file__ + ".result")

def set_result():
    result = "fail"
    timeout = 60

    for i in range(timeout):
        out = cli.get_ibofos_info()
        data = json.loads(out)
        print ("situation: " + data['Response']['info']['situation'])

        if data['Response']['info']['situation'] == "REBUILDING":
            result = "pass"
            break
        else:
            time.sleep(1)
            print("waiting for rebuild starting : " + str(i) + "sec")

    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + "\n" + out)

def execute():
    clear_result()
    ibofos_util.pci_rescan()
    MOUNT_VOL_BASIC_1.execute()
    ibofos_util.pci_detach_and_attach(DETACH_TARGET_DATA)
    time.sleep(6) #TO TRIGGER REBUILD
    ibofos_util.pci_detach_and_attach(DETACH_TARGET_SPARE)
    time.sleep(3)
    print(cli.list_device())
    out = cli.add_device(NEW_SPARE, "")

if __name__ == "__main__":
    execute()
    set_result()
    # ibofos.kill_ibofos()
    # ibofos_util.pci_rescan()