import fio
import initiator
import json
import lib
import target
import graph
from datetime import datetime


def play(json_targets, json_inits, json_scenario):
    lib.printer.green(f"\n -- '{__name__}' has began --")

    raw_date = datetime.now()
    now_date = raw_date.strftime("%y%m%d_%H%M%S")
    skip_workload = False

    # validate arguments, 인자로 받은 json 정보가 있는지 확인
    if 0 == len(json_targets):
        lib.printer.red(" TargetError: At least 1 target has to exist")
        return
    if 0 == len(json_inits):
        lib.printer.red(" InitiatorError: At least 1 initiator has to exist")
        return

    # target prepare, validation check 및 bringup/setup 진행
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

    # init prepare, validation check 및 bringup/setup 진행
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

    # check auto generate
    test_target = targets[next(iter(targets))]
    if "yes" != test_target.use_autogen:
        lib.printer.red(
            f"{__name__} [Error] check [TARGET][AUTO_GENERATE][USE] is 'yes' ")
        skip_workload = True

    # run workload
    if not skip_workload:
        lib.printer.green(f" fio start")

        # [0] sequential fill, volume 용량만큼 data를 채운다
        fio_cmdset = []
        for key in initiators:
            init = initiators[key]
            test_fio = fio.manager.Fio(init.id, init.pw, init.nic_ssh)
            test_fio.opt["ioengine"] = f"{init.spdk_dir}/examples/nvme/fio_plugin/fio_plugin"
            test_fio.opt["size"] = "100%"
            test_fio.opt["time_based"] = "0"
            test_fio.opt["numjobs"] = "1"
            test_fio.opt["thread"] = "1"
            test_fio.opt["group_reporting"] = "1"
            test_fio.opt["direct"] = "1"
            test_fio.opt["readwrite"] = "write"
            test_fio.opt["bs"] = "128k"
            test_fio.opt["iodepth"] = "16"
            test_fio.opt["eta"] = "always"
            test_fio.opt["output"] = f"{init.output_dir}/0_seq_fill_{init.name}"
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
                print(f" run -> {now_date}_fio_seq_fill")
                fio.manager.parallel_run(fio_cmdset)
            except Exception as e:
                lib.printer.red(f"{__name__} [Error] {e}")
                skip_workload = True

        # [1~4] workload
        testcase = [
            ["read_100%_4k", "100", "4k"],
            ["read_70%_4k", "70", "4k"],
            ["read_70%_4k128k", "70", "4k,128k"],
            ["read_0%_4k", "0", "4k"]
        ]
        iodepth = ["1", "4", "16", "32"]
        for tc in testcase:
            # tc 마다 새로운 graph(png 파일)를 그리기 위해 여기에서 graph.manager.Fio 객체를 생성한다
            graph_fio = graph.manager.Fio(
                f"{json_scenario['OUTPUT_DIR']}/{now_date}_1ms_qos_{tc[0]}")
            for qd in iodepth:  # tc 는 iodepth list에 저장되어 있는 qd 만큼의 test를 반복한다
                fio_cmdset = []  # 두 initiator에서 동시에 fio를 수행하기 위해 list로 초기화
                output_name = f"{now_date}_fio_{tc[0]}_qd{qd}"
                for key in initiators:
                    init = initiators[key]
                    test_fio = fio.manager.Fio(init.id, init.pw, init.nic_ssh)
                    test_fio.opt["ioengine"] = f"{init.spdk_dir}/examples/nvme/fio_plugin/fio_plugin"
                    test_fio.opt["ramp_time"] = "15"
                    test_fio.opt["time_based"] = "1"
                    test_fio.opt["runtime"] = "60"
                    test_fio.opt["numjobs"] = "1"
                    test_fio.opt["thread"] = "1"
                    # graph는 group_reporting만 현재 지원 (필수)
                    test_fio.opt["group_reporting"] = "1"
                    test_fio.opt["direct"] = "1"
                    test_fio.opt["readwrite"] = "randrw"
                    test_fio.opt["rwmixread"] = tc[1]
                    test_fio.opt["bs"] = tc[2]
                    test_fio.opt["iodepth"] = qd
                    # eta graph 그리려면 해당 옵션이 필요 (필수)
                    test_fio.opt["eta"] = "always"
                    # 결과를 parsing하기 위한 옵셥 (필수)
                    test_fio.opt["output-format"] = "json"
                    test_fio.opt["output"] = f"{init.output_dir}/{output_name}_{init.name}"
                    for subsys in test_target.subsystem_list:
                        if subsys[0] == init.name:
                            test_fio.jobs.append(
                                f" --name=job_{subsys[2]} --filename=\"trtype={test_target.spdk_tp} adrfam=IPv4 traddr={subsys[3]} trsvcid={subsys[4]} subnqn={subsys[1]} ns=1\"")
                        if not test_fio.Prepare():
                            skip_workload = True
                            break
                    # 각 initiator 마다 설정된 fio 정보를 저장
                    fio_cmdset.append(test_fio.cmd)
                if not skip_workload:
                    try:  # 완성된 fio_cmd들을 병렬수행
                        print(f" run -> {now_date}_fio_{tc[0]}_qd{qd}")
                        fio.manager.parallel_run(fio_cmdset)
                    except Exception as e:
                        lib.printer.red(f"{__name__} [Error] {e}")
                        skip_workload = True
                    try:  # fio 완료 후, 결과를 output directory로 copy
                        for key in initiators:
                            init = initiators[key]
                            lib.subproc.sync_run(
                                f"sshpass -p {init.pw} scp {init.id}@{init.nic_ssh}:{init.output_dir}/{output_name}_{init.name}.eta {json_scenario['OUTPUT_DIR']}")
                            lib.subproc.sync_run(
                                f"sshpass -p {init.pw} scp {init.id}@{init.nic_ssh}:{init.output_dir}/{output_name}_{init.name} {json_scenario['OUTPUT_DIR']}")
                    except Exception as e:
                        lib.printer.red(f"{__name__} [Error] {e}")
                        skip_workload = True
                    try:  # 결과를 add(parsing) & graph로 draw
                        for key in initiators:
                            init = initiators[key]
                            graph_fio.AddEtaData(
                                f"{json_scenario['OUTPUT_DIR']}/{output_name}_{init.name}.eta", f"qd{qd}_{init.name}")
                            graph_fio.DrawEta(
                                ["bw_read", "bw_write", "iops_read", "iops_write"])
                            graph_fio.AddResultData(
                                f"{json_scenario['OUTPUT_DIR']}/{output_name}_{init.name}", f"qd{qd}_{init.name}")
                            graph_fio.DrawResult()
                    except Exception as e:
                        lib.printer.red(f"{__name__} [Error] {e}")
                        skip_workload = True

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
