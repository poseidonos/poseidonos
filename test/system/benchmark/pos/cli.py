import subprocess
import copy
import lib


def system_stop(cmd, cli, dir):
    try:
        cli_cmd = cmd + f"nohup yes | {dir}/bin/{cli} system stop --force"
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
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} devel resetmbr"
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


def bdev_malloc_create(cmd, cli, dir, dev_name, dev_type, num_blk, blk_size, numa):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} device create --device-name {dev_name} --device-type {dev_type} --num-blocks {num_blk} --block-size {blk_size} --numa {numa}"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def transport_create(cmd, cli, dir, trtype, num_shared_buf):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} subsystem create-transport --trtype {trtype} -c 64 --num-shared-buf {num_shared_buf}"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_create(cmd, cli, dir, nqn, sn):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} subsystem create --subnqn {nqn} --serial-number {sn} --model-number POS_VOLUME_EXTENSION -m 256 -o"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_add_listener(cmd, cli, dir, nqn, trtype, ip, port):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} subsystem add-listener --subnqn {nqn} -t {trtype} -i {ip} -p {port}"
        lib.subproc.popen(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_list(cmd, cli, dir):
    try:
        cli_cmd = cmd + f"nohup {dir}/bin/{cli} subsystem list"
        return lib.subproc.popen(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1
