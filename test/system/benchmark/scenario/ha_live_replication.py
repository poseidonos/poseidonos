import graph
import iogen
import lib
import node
import traceback
import random
import string
import os
import subprocess


def validate_data():
    print("* Try to validate primary / secondary data")
    result = subprocess.run(
        "dd if=/dev/nvme0n1 of=primary_data.bin bs=1024", shell=True, stdout=subprocess.PIPE)
    result = subprocess.run(
        "dd if=/dev/nvme1n1 of=secondary_data.bin bs=1024", shell=True, stdout=subprocess.PIPE)
    result = subprocess.run(
        "hexdump -C primary_data.bin > primary_data.hex", shell=True, stdout=subprocess.PIPE)
    result = subprocess.run(
        "hexdump -C secondary_data.bin > secondary_data.hex", shell=True, stdout=subprocess.PIPE)


def clean_up(inits):
    lib.subproc.sync_run("pkill -9 redis")
    lib.subproc.sync_run("pkill -9 replicator")
    lib.subproc.sync_run("rm -rf /var/log/pos/*")
    result = subprocess.run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_primary", shell=True, stdout=subprocess.PIPE)
    result = subprocess.run("umount /remote_home/chkang0912/ibofos/test/system/ha/pos_secondary", shell=True, stdout=subprocess.PIPE)
    for init_dict in inits:
        for subsys in init_dict["TARGETs"][0]["SUBSYSTEMs"]:
            nqn_index = subsys["NQN_INDEX"]
            nqn = f"{subsys['NQN_PREFIX']}{nqn_index:03d}"
            lib.subproc.sync_run(f"nvme disconnect -n {nqn} ")


def play(tgts, inits, scenario, timestamp, data):
    clean_up(inits)
    for init_dict in inits:
        for subsys in init_dict["TARGETs"][0]["SUBSYSTEMs"]:
            nqn_index = subsys["NQN_INDEX"]
            nqn = f"{subsys['NQN_PREFIX']}{nqn_index:03d}"
            lib.subproc.sync_run(f"nvme disconnect -n {nqn} ")

    try:  # Prepare sequence
        node_manager = node.NodeManager(tgts, inits)
        targets, initiators = node_manager.initialize()
        grapher = graph.manager.Grapher(scenario, timestamp)

    except Exception as e:
        lib.printer.red(traceback.format_exc())
        return data

    try:  # Fill first pattern before Volume copy
        bs_list = ["4k"]
        rw_list = ["write"]
        test_case_list = []
        test_case_num = 1
        for bs in bs_list:
            for rw in rw_list:
                pattern = '\\\"#' + \
                    ''.join(random.choice(string.ascii_uppercase + string.digits)
                            for _ in range(14)) + '#\\\"'
                lib.printer.green(f" using pattern: {pattern}")

                test_case = {}
                test_case["name"] = f"Origin_{test_case_num:02d}_{bs}_{rw}"
                test_case["bs"] = bs
                test_case["rw"] = rw
                test_case["size"] = "10M"
                test_case["verify"] = "pattern"
                test_case["verify_pattern"] = pattern
                test_case_num += 1
                test_case_list.append(test_case)
        for test_case in test_case_list:
            # setup fio_cmd
            fio_cmd_list = []
            for key in initiators:
                if key == "Initiator01":
                    fio_cmd = iogen.fio.Fio(initiators[key], timestamp)
                    fio_cmd.initialize()
                    fio_cmd.update(test_case)
                    fio_cmd_list.append(fio_cmd.stringify())

            # run fio
            lib.printer.green(f" run -> {timestamp} {test_case['name']}")
            lib.subproc.sync_parallel_run(fio_cmd_list, True)

            # copy output
            init_key = "Initiator01"
            for key in initiators:
                if key == "Initiator01":
                    initiators[key].copy_output(
                        timestamp, test_case["name"], scenario["OUTPUT_DIR"])

            grapher.draw(initiators[init_key], test_case["name"])
    except Exception as e:
        lib.printer.red(traceback.format_exc())
        input("Bringup fail")

    input("wait replicator")
    try:  # Do extra write during volume copy
        bs_list = ["4k"]
        rw_list = ["write"]
        pattern = '\\\"#' + \
            ''.join(random.choice(string.ascii_uppercase + string.digits)
                    for _ in range(14)) + '#\\\"'
        lib.printer.green(f" using pattern: {pattern}")
        test_case_list = []
        test_case_num = 1
        for bs in bs_list:
            for rw in rw_list:
                test_case = {}
                test_case["name"] = f"LiveReplication_{test_case_num:02d}_{bs}_{rw}"
                test_case["bs"] = bs
                test_case["rw"] = rw
                test_case["io_size"] = "10M"
                test_case["verify"] = "pattern"
                test_case["verify_pattern"] = pattern
                test_case_num += 1
                test_case_list.append(test_case)
        for test_case in test_case_list:
            # setup fio_cmd
            fio_cmd_list = []
            for key in initiators:
                if key == "Initiator01":
                    fio_cmd = iogen.fio.Fio(initiators[key], timestamp)
                    fio_cmd.initialize()
                    fio_cmd.update(test_case)
                    fio_cmd_list.append(fio_cmd.stringify())

            # run fio
            lib.printer.green(f" run -> {timestamp} {test_case['name']}")
            lib.subproc.sync_parallel_run(fio_cmd_list, True)

            # copy output
            init_key = "Initiator01"
            for key in initiators:
                if key == "Initiator01":
                    initiators[key].copy_output(
                        timestamp, test_case["name"], scenario["OUTPUT_DIR"])

            grapher.draw(initiators[init_key], test_case["name"])
    except Exception as e:
        lib.printer.red(traceback.format_exc())

    input("Wait for volume copy finish")
    validate_data()

    return data
