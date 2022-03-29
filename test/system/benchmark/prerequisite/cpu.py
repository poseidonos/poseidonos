import lib


def Scaling(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Scaling : {opt}")
    num_core = 0
    frequency = "1000000"
    governor = "performance"
    if (not "min" == opt and not "max" == opt):
        lib.printer.red(f"{__name__}.Scaling valid option: min/max")
        return
    if ("min" == opt):
        governor = "ondemand"
    get_num_core = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo grep -c processor /proc/cpuinfo"
    num_core = int(lib.subproc.sync_run(get_num_core))

    get_frequency = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_{opt}_freq"
    frequency = lib.subproc.sync_run(get_frequency)

    for core_index in range(num_core):
        set_frequency = ["sshpass", "-p", pw,
                         "ssh", f"{id}@{ip}", "sudo", "nohup"]
        set_frequency.extend(
            ["echo", frequency, ">", f"/sys/devices/system/cpu/cpu{core_index}/cpufreq/scaling_max_freq"])
        lib.subproc.sync_run(set_frequency, False, False)

        set_governor = ["sshpass", "-p", pw,
                        "ssh", f"{id}@{ip}", "sudo", "nohup"]
        set_governor.extend(
            ["echo", governor, ">", f"/sys/devices/system/cpu/cpu{core_index}/cpufreq/scaling_governor"])
        lib.subproc.sync_run(set_governor, False, False)
