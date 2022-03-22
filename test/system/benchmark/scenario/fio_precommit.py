import fio
import graph
import initiator
import json
import lib
import target
from datetime import datetime


def play(json_targets, json_inits, json_scenario):  # player.py에서 호출하는 method로 이름/인자 형식을 준수!
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
    if 0 == len(json_scenario):
        lib.printer.red(" ScenarioError: At least 1 scenario has to exist")
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

    # run workload
    if not skip_workload:
        lib.printer.green(f" fio start")
        # targets 딕셔너리에 첫 번째 객체 정보를 가져온다
        test_target = targets[next(iter(targets))]
        # initiators 딕셔너리에 첫 번째 객체 정보를 가져온다
        test_init = initiators[next(iter(initiators))]
        test_fio = fio.manager.Fio(
            test_init.id, test_init.pw, test_init.nic_ssh)  # fio 객체 생성
        graph_fio = graph.manager.Fio(
            f"{json_scenario['OUTPUT_DIR']}/{now_date}_precommit_fio")  # graph 객체 생성

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
        test_fio.opt["direct"] = "1"
        test_fio.opt["eta"] = "always"          # eta graph를 그리려면 해당 옵션이 필요
        test_fio.opt["group_reporting"] = "1"   # 현재는 group_reporting만 지원
        test_fio.opt["output-format"] = "json"  # 결과를 parsing 하기 위한 필수 옵션
        # write_xxx_log option이 동작하기 위한 필수 옵션
        test_fio.opt["per_job_logs"] = "1"
        # write_xxx_log의 첫 번째 열이 epoch 시간으로 출력
        test_fio.opt["log_unix_epoch"] = "1"
        # write_xxx_log가 출력되는 주기는 1초로 설정
        test_fio.opt["log_avg_msec"] = "1000"

        # test할 job을 --name --filename으로 추가할 수 있다. 주로 multi-device를 동시에 실행시키는 용도로 사용된다
        # 해당 option은 test_fio 객체에 jobs 리스트에 아래와 같이 추가한다
        for subsys in test_target.json["SPDK"]["SUBSYSTEMs"]:
            test_fio.jobs.append(f" --name=job_{subsys['SN']} --filename=\"trtype={test_target.spdk_tp} adrfam=IPv4 \
            traddr={test_target.json['NIC'][subsys['IP']]} trsvcid={subsys['PORT']} subnqn={subsys['NQN']} ns=1\"")

        # test마다 가변적인 옵션이 필요할 경우 아래와 같이 loop를 통해 변경해 줄 수 있다
        # 이때 주의할 점은 fio의 특정 옵션은 다른 옵션들과 동시에 사용할 수 없기 때문에 사용자가 주의해서 사용해야 한다
        block_size = ["512", "4k", "128k", "512-128k"]
        readwrite = ["write", "randwrite", "randrw"]
        for bs in block_size:
            if skip_workload:
                break
            for rw in readwrite:
                test_fio.opt["bs"] = bs
                test_fio.opt["readwrite"] = rw
                test_fio.opt["output"] = f"{test_init.output_dir}/{now_date}_precommit_fio_{bs}_{rw}"
                # job별로 구분되어 저장
                test_fio.opt["write_bw_log"] = f"{test_fio.opt['output']}"
                # job별로 구분되어 저장
                test_fio.opt["write_iops_log"] = f"{test_fio.opt['output']}"
                # job별로 구분되어 저장
                test_fio.opt["write_lat_log"] = f"{test_fio.opt['output']}"

                if "randrw" == rw or "512-128k" == bs:
                    if test_fio.opt.get("norandommap"):
                        del test_fio.opt["norandommap"]
                else:
                    test_fio.opt["norandommap"] = "1"
                if "512-128k" == bs:
                    test_fio.opt["bsrange"] = bs
                else:
                    if test_fio.opt.get("bsrange"):
                        del test_fio.opt["bsrange"]

                if not test_fio.Prepare():  # 성공하면 test_fio 객체에 fio를 수행할 cmd가 완성된다
                    skip_workload = True
                    break

                try:  # 완성된 cmd들을 parallel_run에 list 인자로 전달하면 병렬적으로 test를 동시에 수행할 수 있다
                    print(f" run -> {now_date}_precommit_fio_{bs}_{rw}")
                    fio.manager.parallel_run([test_fio.cmd])
                except Exception as e:
                    lib.printer.red(f"{__name__} [Error] {e}")
                    skip_workload = True
                    break

                try:  # fio 결과를 지정한 output directory로 copy한다
                    lib.subproc.sync_run(
                        f"sshpass -p {test_init.pw} scp {test_init.id}@{test_init.nic_ssh}:{test_fio.opt['output']}* {json_scenario['OUTPUT_DIR']}")
                except Exception as e:
                    lib.printer.red(f"{__name__} [Error] {e}")
                    skip_workload = True
                    break

                # eta data를 graph_fio 객체에 parsing하여 저장
                graph_fio.AddEtaData(
                    f"{test_fio.opt['output']}.eta", f"{bs}_{rw}")
                # 저장되어 있는 eta data를 기반으로 eta graph 생성 및 저장(f"{graph_fio.pic_name}_eta.png")
                # 여기서는(for loop) 매 fio가 끝날 때 마다 graph 갱신, 나중에 한 번만 그리려먼 for loop 밖에서 호출
                graph_fio.DrawEta(
                    ["bw_read", "bw_write", "iops_read", "iops_write"])

                # result data를 graph_fio 객체에 parsing 하여 저장
                graph_fio.AddResultData(test_fio.opt['output'], f"{bs}_{rw}")
                # 저장되어 있는 result data를 기반으로 result graph 생성 및 저장(f"{graph_fio.pic_name}_result.png")
                # 여기서는(for loop) 매 fio가 끝날 때 마다 graph 갱신, 나중에 한 번만 그리려먼 for loop 밖에서 호출
                graph_fio.DrawResult()

                # log data를 graph_fio 객체에 parsing 하여 저장
                graph_fio.AddLogData(
                    json_scenario['OUTPUT_DIR'], f"{now_date}_precommit_fio_{bs}_{rw}")
                # 저장되어 있는 log data를 기반으로 log graph를 생성 및 저장(f""{graph_name}_bw/iops/clat.png")
                # 해당 graph는 다른 test가 누적되어 그려지지 않음, 하나의 fio process(jobs)가 끝날때 마다 새로운 graph(png) 생성
                graph_fio.DrawLog(
                    f"{json_scenario['OUTPUT_DIR']}/{now_date}_precommit_fio_{bs}_{rw}")
                # 다시 해당 객체를 재사용해서 새로운 graph를 그려줄 예정이기 때문에 이전 log data를 clear
                graph_fio.ClearLogData()

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
