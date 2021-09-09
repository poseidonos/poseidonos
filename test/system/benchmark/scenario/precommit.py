import json
import lib
import target
import initiator
import fio
from datetime import datetime


def play(json_targets, json_inits, json_scenario):
    lib.printer.green(f"\n -- '{__name__}' has began --")

    raw_date = datetime.now()
    now_date = raw_date.strftime("%y%m%d_%H%M%S")
    skip_workload = False

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
        if not target_obj.Prepare():
            skip_workload = True
            break
        targets[target_name] = target_obj

    # init prepare
    initiators = {}
    for json_init in json_inits:
        try:
            init_obj = initiator.manager.Initiator(json_init)
            init_name = json_init["NAME"]
        except KeyError:
            lib.printer.red(" InitiatorError: Initiator KEY is invalid")
            return
        if not init_obj.Prepare():
            skip_workload = True
            break
        initiators[init_name] = init_obj
    # run workload
    if not skip_workload:
        lib.printer.green(f" fio start")
        test_target = targets["Target01"]
        test_init = initiators["Initiator01"]
        test_fio = fio.manager.Fio()

        test_fio.cmd = test_init.fio_cmd

        test_fio.opt["ioengine"] = f"{test_init.spdk_dir}/examples/nvme/fio_plugin/fio_plugin"
        test_fio.opt["runtime"] = "120"
        test_fio.opt["io_size"] = "10m"
        test_fio.opt["iodepth"] = "128"
        test_fio.opt["verify"] = "md5"
        test_fio.opt["serialize_overlap"] = "1"
        test_fio.opt["time_based"] = "0"
        test_fio.opt["ramp_time"] = "0"
        test_fio.opt["numjobs"] = "1"
        test_fio.opt["thread"] = "1"
        test_fio.opt["group_reporting"] = "1"
        test_fio.opt["direct"] = "1"
        # test_fio.opt["output-format"] = "json"

        test_fio.jobs.append(f" --name=precommit_test_01 --filename=\"trtype={test_target.spdk_tp} adrfam=IPv4 \
        traddr={test_target.nic_ip1} trsvcid=1158 subnqn=nqn.2020-10.pos\\:subsystem01 ns=1\"")
        test_fio.jobs.append(f" --name=precommit_test_02 --filename=\"trtype={test_target.spdk_tp} adrfam=IPv4 \
        traddr={test_target.nic_ip1} trsvcid=1158 subnqn=nqn.2020-10.pos\\:subsystem02 ns=1\"")
        test_fio.jobs.append(f" --name=precommit_test_03 --filename=\"trtype={test_target.spdk_tp} adrfam=IPv4 \
        traddr={test_target.nic_ip1} trsvcid=1158 subnqn=nqn.2020-10.pos\\:subsystem03 ns=1\"")

        block_size = ["512", "4k", "128k", "512-128k"]
        readwrite = ["write", "randwrite", "randrw"]
        for bs in block_size:
            for rw in readwrite:
                test_fio.opt["bs"] = bs
                test_fio.opt["readwrite"] = rw
                test_fio.opt["output"] = f"{test_init.output_dir}/{now_date}_precommit_fio_{bs}_{rw}"

                if "randrw" == rw:
                    if test_fio.opt.get("norandommap"):
                        del test_fio.opt["norandommap"]
                else:
                    test_fio.opt["norandommap"] = "1"
                if "512-128k" == bs:
                    test_fio.opt["bsrange"] = bs
                else:
                    if test_fio.opt.get("bsrange"):
                        del test_fio.opt["bsrange"]

                if test_fio.SyncRun():
                    print(f" {now_date}_precommit_fio_{bs}_{rw} done")
                    try:
                        lib.subproc.popen(f"sshpass -p {test_init.pw} scp {test_init.id}@{test_init.nic_ssh}:{test_fio.opt['output']} {json_scenario['OUTPUT_DIR']}")
                    except Exception as e:
                        lib.printer.red(f"{__name__} [Error] {e}")
                else:
                    skip_workload = True
                    break
            if skip_workload:
                break

        lib.printer.green(f" fio end")

    # init wrapup
    for key in initiators:
        initiators[key].Wrapup()

    # target warpup
    for key in targets:
        if not targets[key].Wrapup():
            targets[key].ForcedExit()

    if skip_workload:
        lib.printer.red(f" -- '{__name__}' unexpected done --\n")
    else:
        lib.printer.green(f" -- '{__name__}' successfully done --\n")
