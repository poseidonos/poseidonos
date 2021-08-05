import copy
import lib

rpc = "/scripts/rpc.py"


def nvmf_create_transport(cmd, dir, tp, no_shd_buf):
    try:
        rpc_cmd = cmd + f"nohup {dir}{rpc} nvmf_create_transport -t {tp} -b 64 -n {no_shd_buf}"
        lib.subproc.popen(rpc_cmd)
        return 0
    except Exception as e:
        lib.printer.red(rpc_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def bdev_malloc_create(cmd, dir, write_buf_size_in_mb):
    try:
        rpc_cmd = cmd + f"nohup {dir}{rpc} bdev_malloc_create -b uram0 {write_buf_size_in_mb} 512"
        lib.subproc.popen(rpc_cmd)
        return 0
    except Exception as e:
        lib.printer.red(rpc_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def nvmf_create_subsystem(cmd, dir, nqn, sn):
    try:
        rpc_cmd = cmd + f"nohup {dir}{rpc} nvmf_create_subsystem {nqn} -m 256 -a -s {sn} -d POS_VOLUME_EXTENTION"
        lib.subproc.popen(rpc_cmd)
        return 0
    except Exception as e:
        lib.printer.red(rpc_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def nvmf_subsystem_add_listener(cmd, dir, nqn, tp, ip, port):
    try:
        rpc_cmd = cmd + f"nohup {dir}{rpc} nvmf_subsystem_add_listener {nqn} -t {tp} -a {ip} -s {port}"
        lib.subproc.popen(rpc_cmd)
        return 0
    except Exception as e:
        lib.printer.red(rpc_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def nvmf_get_subsystems(cmd, dir):
    try:
        rpc_cmd = cmd + f"nohup {dir}{rpc} nvmf_get_subsystems"
        return lib.subproc.popen(rpc_cmd)
    except Exception as e:
        lib.printer.red(rpc_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1
