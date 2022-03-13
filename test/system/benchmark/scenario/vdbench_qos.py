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

        workload_list = [
            "seq_w",
            "seq_r",
            "rand_w",
            "rand_r"
        ]
        rd_list = [
            r"seq_w,wd=seq,iorate=max,elapsed=18,interval=3,warmup=3,pause=5,forxfersize=\(128k\),forrdpct=\(0\),forthreads=\(128\)",
            r"seq_r,wd=seq,iorate=max,elapsed=18,interval=3,warmup=3,pause=5,forxfersize=\(128k\),forrdpct=\(100\),forthreads=\(128\)",
            r"rand_w,wd=rand,iorate=max,elapsed=18,interval=3,warmup=3,pause=5,forxfersize=\(4k\),forrdpct=\(0\),forthreads=\(512\)",
            r"rand_r,wd=rand,iorate=max,elapsed=18,interval=3,warmup=3,pause=5,forxfersize=\(4k\),forrdpct=\(100\),forthreads=\(512\)"
        ]
        tc_list = [
            # {
            #    "title": "Reset Throttling",
            #    "sc_list": [["reset", "", ""]]
            # },
            # {
            #    "title": "Reset Throttling",
            #    "sc_list": [["bw", "value", "100"], ["reset", "", ""]]
            # },
            {
               "title": "",
               "sc_list": [["bw", ["1"], ["800"], "min"], ["bw", ["1"], ["1000"], "min"], ["bw", ["1"], ["1500"], "min"], ["reset", "", "", "min"]]
            },
            {
                "title": "Throttle Max BW to 10% of Base Performance",
                "sc_list": [["bw", "rate", "10"]]
            },
            {
                "title": "Throttle Max IOPS to 10% of Base Performance",
                "sc_list": [["iops", "rate", "10"]]
            },
            {
                "title": "Throttle Max BW to 50% of Base Performance",
                "sc_list": [["bw", "rate", "50"]]
            },
            {
                "title": "Throttle Max IOPS to 50% of Base Performance",
                "sc_list": [["iops", "rate", "50"]]
            },
            {
                "title": "Throttle Max BW 90% of Base Performance",
                "sc_list": [["bw", "rate", "90"]]
            },
            {
                "title": "Throttle MAX IOPS 90% of Base Performance",
                "sc_list": [["iops", "rate", "90"]]
            },
            {
                "title": "Throttle Max BW 150% of Base Performance",
                "sc_list": [["bw", "rate", "150"]]
            },
            {
                "title": "Throttle Max IOPS 150% of Base Performance",
                "sc_list": [["iops", "rate", "150"]]
            },
            {
                "title": "Throttle Max BW to Min Performance",
                "sc_list": [["bw", "value", "10"]]
            },
            {
                "title": "Throttle Max IOPS to Min Performance",
                "sc_list": [["iops", "value", "10"]]
            },
            {
                "title": "Throttle Both Max BW and Iops",
                "sc_list": [["bw", "rate", "20"], ["iops", "rate", "50"], ["bw", "rate", "50"], ["iops", "rate", "30"]]
            },
            {
                "title": "Throttle Each Volume with Different Value",
                "sc_list": [["bw", ["3"], ["50"]], ["iops", ["1-2", "4-5"], ["10", "20"]], ["bw", ["3"], ["50"]]]
            }
         ]

        first_init_key = list(initiators.keys())[0]
        first_init = initiators[first_init_key]

        # create vd file & run
        first_init_vdbench = vdbench.manager.Vdbench(first_init.name, first_init.id, first_init.pw, first_init.nic_ssh, first_init.vdbench_dir, json_scenario['OUTPUT_DIR'])
        first_init_vdbench.opt["size"] = "8g"
        first_init_vdbench.CreateVdFile(initiators, rd_list, -1, True)
        first_init_vdbench.run(True)
        first_init_vdbench.CopyVdbenchTotalResult(True, workload_list)

        base_perf = {}
        base_perf = first_init_vdbench.GetBasePerformance(workload_list)  # iops, MB/s

        # run each test for each workload
        # make vd file with only 1 workload
        workload_count = len(workload_list)
        error_return = False

        for rd_idx in range(0, workload_count):
            workload_name = workload_list[rd_idx]

            prev_expected_value = {}  # iops, MB/s
            base_bw = prev_expected_value["bw"] = float(base_perf[workload_name]["bw"])
            base_iops = prev_expected_value["iops"] = float(base_perf[workload_name]["iops"])
            base_iops /= 1000.0  # kiops
            print("\n")
            lib.printer.green(f" === <Base Performance> IOPS: {base_iops} k, BW: {base_bw}MB/sec === ")

            # Create Vdbench File
            print(f" Run: {rd_list[rd_idx]}")
            vd_disk_names = first_init_vdbench.CreateVdFile(initiators, rd_list, rd_idx)
            for tc in tc_list:
                print("\n")
                lib.printer.green(f" **** TEST NAME : {tc['title']} ****")
                for sc in tc["sc_list"]:

                    print(f" {sc}")
                    validty = pos.qos.CheckTestValidity(tc["title"], sc)
                    if validty is False:
                        print(f" Invalid test : {sc}")
                        continue
                    applyAllVolume = (type(sc[1]) is not list)

                    # Get Qos Command Option
                    limit = {}
                    # case1) {"type": , "how": , "value": } case2) {"type: ", "1-2": 10, "4-5": 20}
                    limit = pos.qos.GetQosCommandOption(sc, base_perf[workload_name])

                    # Run Vdbench
                    vdbench_thread = threading.Thread(target=first_init_vdbench.run)
                    vdbench_thread.start()

                    sleep(1)
                    # Set Throttling
                    expected_value = 0
                    if applyAllVolume is True:
                        expected_value = pos.qos.SetQosToAllVolumes(test_target, limit)  # kiops, MB/s
                        if limit["type"] == "iops" and expected_value != -1:
                            expected_value *= 1000
                        if limit["type"] == "reset" or expected_value > base_perf[workload_name][limit["type"]]:
                            expected_value = 0
                    else:
                        pos.qos.SetQosToEachVolumes(test_target, limit)
                    sleep(1)

                    # Wait for vdbench till done
                    vdbench_thread.join(timeout=60)

                    # Check Result
                    throttle_success = False
                    if applyAllVolume is True:
                        first_init_vdbench.CopyVdbenchTotalResult(False, [workload_name])
                        result_file = json_scenario['OUTPUT_DIR'] + "/" + workload_name + ".json"
                        [throttle_success, prev_expected_value] = pos.qos.CheckQos(result_file, limit["type"], expected_value, prev_expected_value, base_perf[workload_name], limit["min"])

                    else:
                        for key in initiators:
                            init = initiators[key]
                            volume_id_list = init.GetVolumeIdOfDevice(vd_disk_names[key])
                            throttle_success = pos.qos.CheckEachVolume(key, limit, vd_disk_names[key], first_init_vdbench, workload_name, volume_id_list, limit["min"])

                    if throttle_success is False:
                        lib.printer.red(f" Failed to QoS")
                        error_return = True
                        break
                    else:
                        lib.printer.green(f" Throttling success")
                    print("")
                    if (error_return is True):
                        break
                if (error_return is True):
                    break
                # Reset Qos After Each Test
                limit = {"type": "reset", "how": "", "value": 0}
                pos.qos.SetQosToAllVolumes(test_target, limit)
                prev_expected_value["bw"] = float(base_perf[workload_name]["bw"])
                prev_expected_value["iops"] = float(base_perf[workload_name]["iops"])
            if (error_return is True):
                break

        lib.printer.green(f" Qos Test With Vdbench End")

    # init wrapup
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
