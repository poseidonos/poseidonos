import asyncio
import graph
import initiator
import json
import lib
import pos
import target
import threading
import vdbench
from datetime import datetime
from time import sleep
from time import time

workload_list = [
    "seq_w"
]
vdbench_list = [
    {"title" : "Write Sequential 128k",
    "workload" : "seq_w",
    "event" : r"seq_w,wd=seq,iorate=max,elapsed=80,interval=1,warmup=3,pause=5,forxfersize=\(128k\),forrdpct=\(0\),forthreads=\(128\)",
    "start" : 3, "duration" : 80}
]

qos_list = [
    {
        "title": "Reset Throttling",
        "reset": True,
        "vol" : "ALL",
        "start" : 5
    },
    {
        "title": "Min Throttling",
        "minbw" : 3200,
        "array" : "0",
        "vol" : "1",
        "start" : 20
    },
    {
        "title": "Min Throttling 0",
        "reset": True,
        "array" : "0",
        "vol" : "1",
        "start" : 50
    },
]


def execute_vdbench_event(first_init_vdbench, initiators, current_time):
    for vdbench_elem in vdbench_list:
        if ("submit" not in vdbench_elem and vdbench_elem["start"] <= current_time):
            print("")
            print(f"Current Time : {current_time}s Run: {vdbench_elem['title']}")
            print("")
            vd_disk_names = first_init_vdbench.CreateVdFile(initiators, [vdbench_elem["event"]], 0)
            vdbench_elem["vd_disk_names"] = vd_disk_names
            vdbench_thread = threading.Thread(target=first_init_vdbench.run)
            vdbench_thread.start()
            vdbench_elem["submit"] = True
            vdbench_elem["thread"] = vdbench_thread


def execute_qos_cli_event(test_target, current_time):
    for qos_cmd in qos_list:
        if ("done" not in qos_cmd and qos_cmd["start"] <= current_time):
            print("")
            print(f"Current Time : {current_time}s Run: {qos_cmd['title']}")
            print("")
            result = pos.qos.SetQos(test_target, qos_cmd)
            qos_cmd["done"] = True
            if (result == False):
                # test done
                return True
            
    # test done
    return False


def wait_vdbench_event(test_vdbench, initiators, current_time):
    test_done = 0
    result = {}
    for vdbench_elem in vdbench_list:
        if ("done" in vdbench_elem):
            test_done = test_done + 1
            continue
        if ("done" not in vdbench_elem and vdbench_elem["start"] + vdbench_elem["duration"] <= current_time):
            print("")
            print(f"Current Time : {current_time}s Exit: {vdbench_elem['title']}")
            print("")
            vdbench_elem["thread"].join(timeout=60)
            vd_disk_names = vdbench_elem["vd_disk_names"]
            for key in initiators:
                #init = initiators[key]
                #volume_id_list = init.GetVolumeIdOfDevice(vd_disk_names[key])
                result_array = pos.qos.GetResultQos(test_vdbench, key, vd_disk_names[key], "bw", vdbench_elem["workload"])
                result[key] = result_array
            vdbench_elem["done"] = True
            vdbench_elem["result_array"] = result
    if (test_done == len(vdbench_list)):
        result["done"] = "True"
        return result
    else:
        return result
    

def play(json_targets, json_inits, json_scenario):
    lib.printer.green(f"\n -- '{__name__}' has began --")

    raw_date = datetime.now()
    now_date = raw_date.strftime("%y%m%d_%H%M%S")
    skip_workload = False
    lib.subproc.set_allow_stdout()
    # validate arguments
    if 0 == len(json_targets):
        lib.printer.red(" TargetError: At least 1 target has to exist")
        return
    if 0 == len(json_inits):
        lib.printer.red(" InitiatorError: At least 1 initiator has to exist")
        return
    # target prepare
    targets = {}
    for json_target in json_targets:
        try:
            target_obj = target.manager.Target(json_target)
            target_name = json_target["NAME"]
        except KeyError:
            lib.printer.red(" TargetError: Target KEY is invalid")
            return
        target_obj.CliInLocal()
        if not target_obj.Prepare():
            skip_workload = True
            break
        targets[target_name] = target_obj

    # init prepare
    initiators = {}
    test_target = targets[next(iter(targets))]
    for json_init in json_inits:
        try:
            init_obj = initiator.manager.Initiator(json_init)
            init_name = json_init["NAME"]
        except KeyError:
            lib.printer.red(" InitiatorError: Initiator KEY is invalid")
            return
        if not init_obj.Prepare(True, test_target.subsystem_list):
            skip_workload = True
            break
        initiators[init_name] = init_obj

    # check auto generate

    if "yes" != test_target.use_autogen:
        lib.printer.red(f"{__name__} [Error] check [TARGET][AUTO_GENERATE][USE] is 'yes' ")
        skip_workload = True

    if not skip_workload:
        lib.printer.green(f" Qos Test With Vdbench Start")

        first_init_key = list(initiators.keys())[0]
        first_init = initiators[first_init_key]

        # create vd file & run
        first_init_vdbench = vdbench.manager.Vdbench(first_init.name, first_init.id, first_init.pw, first_init.nic_ssh, first_init.vdbench_dir, json_scenario['OUTPUT_DIR'])
        first_init_vdbench.opt["size"] = "8g"

        # run each test for each workload
        # make vd file with only 1 workload
        start_time = time()
        while(1):
            current_time = time() - start_time
            execute_vdbench_event(first_init_vdbench, initiators, current_time)
            result = wait_vdbench_event(first_init_vdbench, initiators, current_time)
            if ("done" in result):
                break
            if (execute_qos_cli_event(test_target, current_time) == True):
                break
        lib.printer.green(f" Qos Test With Vdbench End")
    # init wrapup
    for vdbench_elem in vdbench_list:
        if ("result_array" in vdbench_elem):
            print(vdbench_elem["title"])
            print(result_array)


    for key in initiators:
        initiators[key].Wrapup(True, test_target.subsystem_list)

    # target warpup
    for key in targets:
        if not targets[key].Wrapup():
            targets[key].ForcedExit()

    if skip_workload:
        lib.printer.red(f" -- '{__name__}' unexpected done --\n")
    else:
        lib.printer.green(f" -- '{__name__}' successfully done --\n")
