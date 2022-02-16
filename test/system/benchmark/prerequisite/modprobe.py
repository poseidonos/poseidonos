import lib


def Modprobe(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Modprobe : {opt}")
    for mod in opt:
        try:
            modprobe = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
                sudo modprobe {mod}"
            lib.subproc.sync_run(modprobe)
        except Exception as e:
            lib.printer.red(modprobe)
            lib.printer.red(f"{__name__} [Error] {e}")
            return -1
    return 0
