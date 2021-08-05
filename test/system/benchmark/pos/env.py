import time
import copy
import lib


def check_pos_running(cmd, bin):
    try:
        ps_cmd = cmd + f"ps -e | grep {bin}"
        result = lib.subproc.popen(ps_cmd)
        if -1 == result.find(bin):
            return False
        else:
            return True
    except Exception as e:
        lib.printer.red(ps_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def kill_pos(cmd, bin):
    try:
        pkill_cmd = cmd + f"pkill -9 {bin}"
        lib.subproc.popen(pkill_cmd)
        return 0
    except Exception as e:
        lib.printer.red(pkill_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def copy_pos_config(cmd, dir, cfg):
    try:
        copy_cmd = cmd + f"cp {dir}/config/{cfg} /etc/pos/pos.conf"
        lib.subproc.popen(copy_cmd)
        return 0
    except Exception as e:
        lib.printer.red(copy_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def execute_pos(cmd, bin, dir, log):
    try:
        exe_cmd = copy.deepcopy(cmd)
        exe_cmd.extend(["nohup", f"{dir}/bin/{bin}", "&>>", f"{dir}/script/{log}&"])
        lib.subproc.popen(exe_cmd, False, False)
    except Exception as e:
        lib.printer.red(exe_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1
