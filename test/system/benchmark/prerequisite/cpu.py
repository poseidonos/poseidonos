import lib


def Scaling(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Scaling : {opt}")
    num_core = 0
    frequency = "1000000"
    governor = "performance"
    if (not "min" == opt and not "max" == opt):
        lib.printer.red(f"{__name__}.Scaling valid option: min/max")
        return -1
    if ("min" == opt):
        governor = "ondemand"

    try:
        get_num_core = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo grep -c processor /proc/cpuinfo"
        num_core = int(lib.subproc.sync_run(get_num_core))
    except Exception as e:
        lib.printer.red(get_num_core)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        get_frequency = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_{opt}_freq"
        frequency = lib.subproc.sync_run(get_frequency)
    except Exception as e:
        lib.printer.red(get_frequency)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    for core_index in range(num_core):
        try:
            set_frequency = ["sshpass", "-p", pw,
                             "ssh", f"{id}@{ip}", "sudo", "nohup"]
            set_frequency.extend(
                ["echo", frequency, ">", f"/sys/devices/system/cpu/cpu{core_index}/cpufreq/scaling_max_freq"])
            lib.subproc.sync_run(set_frequency, False, False)
        except Exception as e:
            lib.printer.red(set_frequency)
            lib.printer.red(f"{__name__} [Error] {e}")
            return -1
        try:
            set_governor = ["sshpass", "-p", pw,
                            "ssh", f"{id}@{ip}", "sudo", "nohup"]
            set_governor.extend(
                ["echo", governor, ">", f"/sys/devices/system/cpu/cpu{core_index}/cpufreq/scaling_governor"])
            lib.subproc.sync_run(set_governor, False, False)
        except Exception as e:
            lib.printer.red(set_governor)
            lib.printer.red(f"{__name__} [Error] {e}")
            return -1

    return 0
