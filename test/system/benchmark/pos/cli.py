import lib
import subprocess


def system_stop(id, pw, ip, cli, dir):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} system stop --force"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def device_scan(id, pw, ip, cli, dir):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} device scan"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_reset(id, pw, ip, cli, dir):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} devel resetmbr"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_create(id, pw, ip, cli, dir, buffer_dev, user_devs, spare_devs, arr_name, raid_type):
    try:
        if 0 == len(spare_devs):
            cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} array create -b {buffer_dev} -d {user_devs} --array-name {arr_name} --raid {raid_type}"
        else:
            cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} array create -b {buffer_dev} -d {user_devs} -s {spare_devs} --array-name {arr_name} --raid {raid_type}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_mount(id, pw, ip, cli, dir, arr_name):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} array mount --array-name {arr_name}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def array_unmount(id, pw, ip, cli, dir, arr_name):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} array unmount --array-name {arr_name} --force"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def volume_create(id, pw, ip, cli, dir, vol_name, vol_size, arr_name):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} volume create --volume-name {vol_name} --size {vol_size} --maxiops 0 --maxbw 0 --array-name {arr_name}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def volume_mount(id, pw, ip, cli, dir, vol_name, subnqn, arr_name):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} volume mount --volume-name {vol_name} --array-name {arr_name} --subnqn {subnqn}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def bdev_malloc_create(id, pw, ip, cli, dir, dev_name, dev_type, num_blk, blk_size, numa):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} device create --device-name {dev_name} --device-type {dev_type} --num-blocks {num_blk} --block-size {blk_size} --numa {numa}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def transport_create(id, pw, ip, cli, dir, trtype, num_shared_buf):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} subsystem create-transport --trtype {trtype} -c 64 --num-shared-buf {num_shared_buf}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_create(id, pw, ip, cli, dir, nqn, sn):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} subsystem create --subnqn {nqn} --serial-number {sn} --model-number POS_VOLUME_EXTENSION -m 256 -o"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_add_listener(id, pw, ip, cli, dir, nqn, trtype, target_ip, port):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} subsystem add-listener --subnqn {nqn} -t {trtype} -i {target_ip} -p {port}"
        lib.subproc.sync_run(cli_cmd)
        return 0
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def subsystem_list(id, pw, ip, cli, dir):
    try:
        cli_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo nohup {dir}/bin/{cli} subsystem list"
        return lib.subproc.sync_run(cli_cmd)
    except Exception as e:
        lib.printer.red(cli_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1
