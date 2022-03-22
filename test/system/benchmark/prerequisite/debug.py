import lib


def Ulimit(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Ulimit : {opt}")
    try:
        set_ulimit = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            ulimit -c {opt}"
        lib.subproc.sync_run(set_ulimit)
    except Exception as e:
        lib.printer.red(set_ulimit)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1
    return 0


def Apport(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Apport : {opt}")
    try:
        set_apport = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            /lib/systemd/systemd-sysv-install {opt} apport"
        lib.subproc.sync_run(set_apport)
    except Exception as e:
        lib.printer.red(set_apport)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1
    return 0


def CorePattern(id, pw, ip, core_dir, core_pattern):
    lib.printer.green(
        f" + {__name__}.CorePattern : {core_dir}, {core_pattern}")
    try:
        mk_core_dir = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo mkdir -p {core_dir}"
        lib.subproc.sync_run(mk_core_dir)
    except Exception as e:
        lib.printer.red(mk_core_dir)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        set_core_pattern = ["sshpass", "-p", pw,
                            "ssh", f"{id}@{ip}", "sudo", "nohup"]
        set_core_pattern.extend(
            ["echo", core_pattern, ">", "/proc/sys/kernel/core_pattern"])
        lib.subproc.sync_run(set_core_pattern, False, False)
    except Exception as e:
        lib.printer.red(set_core_pattern)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    return 0
