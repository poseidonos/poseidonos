import graph
import iogen
import json
import lib
import node
import rsfmt
import traceback
import os
import pos
import time

def play(tgts, inits, scenario, timestamp, data):
    try:  # Prepare sequence
        os.system("pkill -9 pos-exporter")
        time.sleep(2)
        os.system("./bin/pos-exporter &")
        os.system("echo 1 > /sys/bus/pci/rescan")
        node_manager = node.NodeManager(tgts, inits)
        targets, initiators = node_manager.initialize()

        test_case_list = [
         #   {"name": "2_sw", "rw": "write", "bs": "128k", "iodepth": "4", "io_size": "100%",
         #       "time_based": "1", "runtime": "15", "log_avg_msec": "30000"},
         #   {"name": "3_sr", "rw": "read", "bs": "128k", "iodepth": "4", "io_size": "1t",
         #       "time_based": "1", "runtime": "20", "log_avg_msec": "2000"},
         #   {"name": "4_rw", "rw": "randwrite", "bs": "4k", "iodepth": "128",
         #       "io_size": "100%", "time_based": "1", "runtime": "15", "log_avg_msec": "2000"},
            {"name": "5_rr", "rw": "randread", "bs": "4k", "iodepth": "128",
               "io_size": "100%", "time_based": "1", "runtime": "15", "log_avg_msec": "20000"},
        ]
        rebuild_impact_list = ["none", "degraded", "low", "medium", "high"]
      #  rebuild_impact_list = ["none"]
        os.system("./bin/poseidonos-cli telemetry start")
        grapher = graph.manager.Grapher(scenario, timestamp)
        result_fmt = rsfmt.manager.Formatter(scenario, timestamp)
        result_fmt.add_test_cases([tc['name'] for tc in test_case_list])
    except Exception as e:
        lib.printer.red(traceback.format_exc())
        return data
    
    try:
        for test_case in [{"name": "1_sw", "rw": "write", "bs": "128k", "iodepth": "4", "io_size": "100g",
            "time_based": "0", "runtime": "0", "log_avg_msec": "30000"}]:
            # setup fio_cmd
            fio_cmd_list = []
            for key in initiators:
                fio_cmd = iogen.fio.Fio(initiators[key], timestamp)
                fio_cmd.initialize()
                fio_cmd.update(test_case)
                fio_cmd_list.append(fio_cmd.stringify())

            # run fio
            lib.printer.green(f" run -> {timestamp} {test_case['name']}")
            result_fmt.start_test(test_case["name"])
            lib.subproc.sync_parallel_run(fio_cmd_list, True)

            # copy output
            for key in initiators:
                initiators[key].copy_output(
                    timestamp, test_case["name"], scenario["OUTPUT_DIR"])

            # get result
            for key in initiators:
                file = (
                    f"{scenario['OUTPUT_DIR']}/{timestamp}_"
                    f"{test_case['name']}_{key}"
                )
                fio_result = lib.parser.parse_json_file(file)
                print(json.dumps(fio_result, indent=2))

            # set result status (& message)
            result_fmt.end_test(test_case["name"], "pass")

            # draw graph
            for key in initiators:
                grapher.draw(initiators[key], test_case["name"])
    except Exception as e:
        lib.printer.red(traceback.format_exc())
    
    try:  # Test sequence
        detached = False
        spare_attached = False
        for rebuild_impact in rebuild_impact_list:
            if (rebuild_impact == "degraded"):
                if (detached == False):
                    for key in targets:
                        targets[key].pcie_scan()
                        ret = targets[key].detach_device("0000:6a:00.0")
                        print("detach success : {}", ret)
                       # time.sleep(3600)
                    detached = True
            if (rebuild_impact != "none" and rebuild_impact != "degraded"):
                if (spare_attached == False):
                    os.system("./bin/poseidonos-cli array addspare --spare unvme-ns-15 --array-name ARR0")
                    print("attach spare")
                    time.sleep(55)
                    spare_attached = True
                print ("rebuild Impact : ", rebuild_impact)
                for key in targets:
                    targets[key].set_rebuild_impact(rebuild_impact)
            for test_case in test_case_list:
                # setup fio_cmd
                fio_cmd_list = []
                for key in initiators:
                    fio_cmd = iogen.fio.Fio(initiators[key], timestamp)
                    fio_cmd.initialize()
                    fio_cmd.update(test_case)
                    fio_cmd_list.append(fio_cmd.stringify())

                # run fio
                lib.printer.green(f" run -> {timestamp} {test_case['name']}")
                result_fmt.start_test(test_case["name"])
                lib.subproc.sync_parallel_run(fio_cmd_list, True)

                # copy output
                for key in initiators:
                    initiators[key].copy_output(
                        timestamp, test_case["name"], scenario["OUTPUT_DIR"])

                # get result
                for key in initiators:
                    file = (
                        f"{scenario['OUTPUT_DIR']}/{timestamp}_"
                        f"{test_case['name']}_{key}"
                    )
                    fio_result = lib.parser.parse_json_file(file)
                    print(json.dumps(fio_result, indent=2))

                # set result status (& message)
                result_fmt.end_test(test_case["name"], "pass")

                # draw graph
                for key in initiators:
                    grapher.draw(initiators[key], test_case["name"])
            
            for key in targets:
                ret = targets[key].check_rebuild_complete("ARR0")
                print("rebuild check : ", ret)
                   

                
    except Exception as e:
        lib.printer.red(traceback.format_exc())

    try:  # Wrapup sequence
        result_fmt.write_file()
        node_manager.finalize()
    except Exception as e:
        lib.printer.red(traceback.format_exc())

    return data
