import fio
import graph
import initiator
import json
import lib
import target
import traceback
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
    if 0 == len(json_scenario):
        lib.printer.red(" ScenarioError: At least 1 scenario has to exist")
        return

    # target prepare
    targets = {}
    for json_target in json_targets:
        try:
            target_obj = target.manager.Target(json_target)
        except Exception as e:
            lib.printer.red(traceback.format_exc())
            return
        target_name = json_target["NAME"]

        try:
            target_obj.Prepare()
        except Exception as e:
            lib.printer.red(traceback.format_exc())
            skip_workload = True
            target_obj.ForcedExit()
            break
        targets[target_name] = target_obj

    # init prepare
    initiators = {}
    for json_init in json_inits:
        try:
            init_obj = initiator.manager.Initiator(json_init)
        except Exception as e:
            lib.printer.red(traceback.format_exc())
            skip_workload = True
            break
        init_name = json_init["NAME"]

        try:
            init_obj.Prepare()
        except Exception as e:
            lib.printer.red(traceback.format_exc())
            skip_workload = True
            break
        initiators[init_name] = init_obj

    # check auto generate
    if not skip_workload:
        test_target = targets[next(iter(targets))]
        if "yes" != test_target.use_autogen:
            lib.printer.red(
                f"{__name__} [Error] check [TARGET][AUTO_GENERATE][USE] is 'yes' ")
            skip_workload = True

    # run workload
    if not skip_workload:
        lib.printer.green(f" fio start")
        graph_fio = graph.manager.Fio(
            f"{json_scenario['OUTPUT_DIR']}/{now_date}_sustained_write")
        # readwrite, block size, io depth
        testcase = [["write", "128k", "4", "seqfill"],
                    ["randwrite", "4k", "128", "randfill1"],
                    ["randwrite", "4k", "128", "randfill2"],
                    ["randrw", "4k", "128", "randwrite"]
                    ]

        for tc in testcase:
            fio_cmdset = []
            rw = tc[0]
            bs = tc[1]
            iodepth = tc[2]
            test_name = tc[3]
            output_name = f"{now_date}_fio_{bs}_{rw}_{test_name}"

            for key in initiators:
                init = initiators[key]
                test_fio = fio.manager.Fio(init.id, init.pw, init.nic_ssh)
                test_fio.opt["ioengine"] = f"{init.spdk_dir}/examples/nvme/fio_plugin/fio_plugin"
                test_fio.opt["runtime"] = "0"
                test_fio.opt["ramp_time"] = "15"
                test_fio.opt["io_size"] = "40g"
                test_fio.opt["verify"] = "0"
                test_fio.opt["size"] = "100%"
                test_fio.opt["serialize_overlap"] = "1"
                test_fio.opt["time_based"] = "0"
                test_fio.opt["numjobs"] = "1"
                test_fio.opt["thread"] = "1"
                test_fio.opt["direct"] = "1"
                if "randrw" == rw:
                    test_fio.opt["rwmixread"] = "0"
                test_fio.opt["readwrite"] = rw
                test_fio.opt["bs"] = bs
                test_fio.opt["iodepth"] = iodepth
                test_fio.opt["eta"] = "always"
                test_fio.opt["group_reporting"] = "1"
                test_fio.opt["output-format"] = "json"

                test_fio.opt["output"] = f"{init.output_dir}/{output_name}_{init.name}"

                if "randwrite" == rw or "randread" == rw:
                    if test_fio.opt.get("norandommap"):
                        del test_fio.opt["norandommap"]
                else:
                    test_fio.opt["norandommap"] = "1"

                if "512-128k" == bs:
                    test_fio.opt["bsrange"] = bs
                else:
                    if test_fio.opt.get("bsrange"):
                        del test_fio.opt["bsrange"]

                for subsys in test_target.subsystem_list:
                    if subsys[0] == init.name:
                        test_fio.jobs.append(
                            f" --name=job_{subsys[2]} --filename=\"trtype={test_target.spdk_tp} adrfam=IPv4 traddr={subsys[3]} trsvcid={subsys[4]} subnqn={subsys[1]} ns=1\"")
                        if not test_fio.Prepare():
                            skip_workload = True
                            break

                fio_cmdset.append(test_fio.cmd)

            if not skip_workload:
                try:
                    print(f" run -> {now_date}_fio_{bs}_{rw}")
                    fio.manager.parallel_run(fio_cmdset)
                except Exception as e:
                    lib.printer.red(f"{__name__} [Error] {e}")
                    skip_workload = True

                try:
                    for key in initiators:
                        init = initiators[key]
                        lib.subproc.sync_run(
                            f"sshpass -p {init.pw} scp {init.id}@{init.nic_ssh}:{init.output_dir}/{output_name}_{init.name}.eta {json_scenario['OUTPUT_DIR']}")
                        lib.subproc.sync_run(
                            f"sshpass -p {init.pw} scp {init.id}@{init.nic_ssh}:{init.output_dir}/{output_name}_{init.name} {json_scenario['OUTPUT_DIR']}")
                except Exception as e:
                    lib.printer.red(f"{__name__} [Error] {e}")
                    skip_workload = True

                try:
                    for key in initiators:
                        init = initiators[key]
                        graph_fio.AddEtaData(
                            f"{json_scenario['OUTPUT_DIR']}/{output_name}_{init.name}.eta", f"{bs}_{rw}")
                        graph_fio.DrawEta(
                            ["bw_read", "bw_write", "iops_read", "iops_write"])
                        graph_fio.AddResultData(
                            f"{json_scenario['OUTPUT_DIR']}/{output_name}_{init.name}", f"{bs}_{rw}")
                        graph_fio.DrawResult()
                except Exception as e:
                    lib.printer.red(f"{__name__} [Error] {e}")
                    skip_workload = True

        lib.printer.green(f" fio end")

    # init wrapup
    for key in initiators:
        try:
            initiators[key].Wrapup()
        except Exception as e:
            lib.printer.red(traceback.format_exc())
            skip_workload = True

    # target warpup
    for key in targets:
        try:
            targets[key].Wrapup()
        except Exception as e:
            lib.printer.red(traceback.format_exc())
            targets[key].ForcedExit()
            skip_workload = True

    if skip_workload:
        lib.printer.red(f" -- '{__name__}' unexpected done --\n")
    else:
        lib.printer.green(f" -- '{__name__}' successfully done --\n")
