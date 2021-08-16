import subprocess
import copy
import lib


def system_stop(cmd, cli, dir):
    try:
        cli_cmd = cmd + f"nohup yes | {dir}/bin/{cli} system stop"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def device_scan(cmd, cli, dir):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} device scan"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_reset(cmd, cli, dir):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} dev resetmbr"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_create(cmd, cli, dir, user_devs, spare_devs, arr_name, raid_type):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} array create -b uram0 -d {user_devs} -s {spare_devs} --array-name {arr_name} --raid {raid_type}"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_mount(cmd, cli, dir, arr_name):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} array mount --array-name {arr_name}"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_unmount(cmd, cli, dir, arr_name):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} array unmount --array-name {arr_name} --force"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def volume_create(cmd, cli, dir, vol_name, vol_size, arr_name):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} volume create --volume-name {vol_name} --size {vol_size} --maxiops 0 --maxbw 0 --array-name {arr_name}"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def volume_mount(cmd, cli, dir, vol_name, subnqn, arr_name):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} volume mount --volume-name {vol_name} --array-name {arr_name} --subnqn {subnqn}"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1
