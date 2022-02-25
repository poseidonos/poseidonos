import lib
import subprocess

execute_cli_in_local = False


def set_cli_in_local():
    global execute_cli_in_local
    execute_cli_in_local = True
    return execute_cli_in_local


def prefix_string(id, pw, ip):
    prefix = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup"
    if (execute_cli_in_local is True):
        prefix = ""
    return prefix


def system_stop(id, pw, ip, cli, dir):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} system stop --force"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def device_scan(id, pw, ip, cli, dir):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} device scan"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_reset(id, pw, ip, cli, dir):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} devel resetmbr"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_create(id, pw, ip, cli, dir, buffer_dev, user_devs, spare_devs, arr_name, raid_type):
    try:
        if 0 == len(spare_devs):
            prefix = prefix_string(id, pw, ip)
            cli_cmd = prefix + \
                f" {dir}/bin/{cli} array create -b {buffer_dev} -d {user_devs} --array-name {arr_name} --raid {raid_type}"
        else:
            prefix = prefix_string(id, pw, ip)
            cli_cmd = prefix + \
                f" {dir}/bin/{cli} array create -b {buffer_dev} -d {user_devs} -s {spare_devs} --array-name {arr_name} --raid {raid_type}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_mount(id, pw, ip, cli, dir, arr_name):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + \
            f" {dir}/bin/{cli} array mount --array-name {arr_name}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_unmount(id, pw, ip, cli, dir, arr_name):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + \
            f" {dir}/bin/{cli} array unmount --array-name {arr_name} --force"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def volume_create(id, pw, ip, cli, dir, vol_name, vol_size, arr_name):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + \
            f" {dir}/bin/{cli} volume create --volume-name {vol_name} --size {vol_size} --maxiops 0 --maxbw 0 --array-name {arr_name}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def volume_mount(id, pw, ip, cli, dir, vol_name, subnqn, arr_name):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + \
            f" {dir}/bin/{cli} volume mount --volume-name {vol_name} --array-name {arr_name} --subnqn {subnqn} --force"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def telemetry_stop(id, pw, ip, cli, dir):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} telemetry stop"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def bdev_malloc_create(id, pw, ip, cli, dir, dev_name, dev_type, num_blk, blk_size, numa):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + \
            f" {dir}/bin/{cli} device create --device-name {dev_name} --device-type {dev_type} --num-blocks {num_blk} --block-size {blk_size} --numa {numa}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def transport_create(id, pw, ip, cli, dir, trtype, num_shared_buf):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + \
            f" {dir}/bin/{cli} subsystem create-transport --trtype {trtype} -c 64 --num-shared-buf {num_shared_buf}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_create(id, pw, ip, cli, dir, nqn, sn):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + \
            f" {dir}/bin/{cli} subsystem create --subnqn {nqn} --serial-number {sn} --model-number POS_VOLUME_EXTENSION -m 256 -o"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_add_listener(id, pw, ip, cli, dir, nqn, trtype, target_ip, port):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + \
            f" {dir}/bin/{cli} subsystem add-listener --subnqn {nqn} -t {trtype} -i {target_ip} -p {port}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_list(id, pw, ip, cli, dir):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} subsystem list"
        return lib.subproc.sync_run(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def logger_setlevel(id, pw, ip, cli, dir, level):
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} logger set-level --level {level}"
        lib.subproc.sync_run(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def check_rebuild_complete(id, pw, ip, cli, dir, arr_name):
    ret = ''
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} array list --array-name {arr_name}"
        ret = lib.subproc.sync_run(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
    return ret


def device_list(id, pw, ip, cli, dir):
    ret = ''
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} device list"
        ret = lib.subproc.sync_run(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
    return ret


def add_spare(id, pw, ip, cli, dir, arr_name, dev_name):
    ret = ''
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} array addspare -a {arr_name} -s {dev_name}"
        ret = lib.subproc.sync_run(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
    return ret


def set_rebuild_impact(id, pw, ip, cli, dir, impact):
    ret = ''
    try:
        prefix = prefix_string(id, pw, ip)
        cli_cmd = prefix + f" {dir}/bin/{cli} system set-property --rebuild-impact {impact}"
        ret = lib.subproc.sync_run(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
    return ret


def set_qos(id, pw, ip, cli, dir, array_name, vol_name, limit_type, limit_value=0, min=False):
    try:
        min_limit_value = 10
        prefix = "max"
        if (min is True):
            prefix = "min"
        limit_value = int(limit_value)
        if (limit_type.lower() != "reset" and limit_value < 10 and limit_value != 0):
            lib.printer.red(f"Cannot throttle {limit_type} to {limit_value} (minimum_limit_value: {min_limit_value}). Qos Command is ignored")
            return -1
        if (limit_type.lower() == "reset"):
            cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} qos reset --array-name {array_name} --volume-name {vol_name}"
        elif (limit_type.lower() == "iops"):
            cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} qos create --array-name {array_name} --volume-name {vol_name} --{prefix}iops {limit_value}"
        else:
            cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} qos create --array-name {array_name} --volume-name {vol_name} --{prefix}bw {limit_value}"
        return lib.subproc.sync_run(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1
