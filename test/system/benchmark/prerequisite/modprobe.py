import lib


def Modprobe(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Modprobe : {opt}")
    for mod in opt:
        modprobe = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo modprobe {mod}"
        lib.subproc.sync_run(modprobe)
