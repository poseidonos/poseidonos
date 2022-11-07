import iogen
import lib
import node
import traceback
import random
import string


def play(tgts, inits, scenario, timestamp, data):
    try:  # Prepare sequence
        node_manager = node.NodeManager(tgts, inits)
        targets, initiators = node_manager.initialize()

    except Exception as e:
        lib.printer.red(traceback.format_exc())
        return data

    try:  # Test sequence
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
                test_case["name"] = f"{test_case_num:02d}_{bs}_{rw}"
                test_case["bs"] = bs
                test_case["rw"] = rw
                test_case["io_size"] = "30m"
                test_case["verify"] = "pattern"
                test_case["verify_pattern"] = pattern
                test_case_num += 1
                test_case_list.append(test_case)
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
            lib.subproc.sync_parallel_run(fio_cmd_list, True)

    except Exception as e:
        lib.printer.red(traceback.format_exc())
        input("bringup complete")
