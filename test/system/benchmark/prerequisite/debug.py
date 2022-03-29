import lib


def Ulimit(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Ulimit : {opt}")
    set_ulimit = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        ulimit -c {opt}"
    lib.subproc.sync_run(set_ulimit)


def Apport(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Apport : {opt}")
    set_apport = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        /lib/systemd/systemd-sysv-install {opt} apport"
    lib.subproc.sync_run(set_apport)


def CorePattern(id, pw, ip, core_dir, core_pattern):
    lib.printer.green(
        f" + {__name__}.CorePattern : {core_dir}, {core_pattern}")
    mk_core_dir = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo mkdir -p {core_dir}"
    lib.subproc.sync_run(mk_core_dir)

    set_core_pattern = ["sshpass", "-p", pw,
                        "ssh", f"{id}@{ip}", "sudo", "nohup"]
    set_core_pattern.extend(
        ["echo", core_pattern, ">", "/proc/sys/kernel/core_pattern"])
    lib.subproc.sync_run(set_core_pattern, False, False)
