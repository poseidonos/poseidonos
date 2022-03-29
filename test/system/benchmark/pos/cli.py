import lib
from pos.cli_version import *


class Cli:
    def __init__(self, json):
        cli_cmd = f"sshpass -p {json['PW']} ssh -o StrictHostKeyChecking=no \
            {json['ID']}@{json['NIC']['SSH']} sudo nohup \
            {json['DIR']}/bin/{json['POS']['CLI']} --version"
        result = lib.subproc.sync_run(cli_cmd)
        if ("poseidonos-cli version 1.0.1" in result):
            self.cli = cli_1_0_1.Cli_1_0_1(json)
        else:
            raise ValueError(f"Not supported poseidonos-cli version: {result}")

    def array_add_spare(self, arr_name, dev_name):
        return self.cli.array_add_spare(arr_name, dev_name)

    def array_create(self, buffer_dev, user_devs, spare_devs, arr_name, raid_type):
        return self.cli.array_create(buffer_dev, user_devs,
                                     spare_devs, arr_name, raid_type)

    def array_list(self, arr_name):
        return self.cli.array_list(arr_name)

    def array_mount(self, arr_name, wb_mode=False):
        return self.cli.array_mount(arr_name, wb_mode)

    def array_reset(self):
        return self.cli.array_reset()

    def array_unmount(self, arr_name):
        return self.cli.array_unmount(arr_name)

    def device_create(self, dev_name, dev_type, num_blk, blk_size, numa):
        return self.cli.device_create(dev_name, dev_type, num_blk, blk_size, numa)

    def device_list(self):
        return self.cli.device_list()

    def device_scan(self):
        return self.cli.device_scan()

    def logger_set_level(self, level):
        return self.cli.logger_set_level(level)

    def qos_create(self, arr_name, vol_name, maxbw, maxiops, minbw, miniops):
        return self.cli.qos_create(arr_name, vol_name, maxbw, maxiops, minbw, miniops)

    def qos_reset(self, arr_name, vol_name):
        return self.cli.qos_reset(arr_name, vol_name)

    def subsystem_add_listener(self, nqn, trtype, target_ip, port):
        return self.cli.subsystem_add_listener(nqn, trtype, target_ip, port)

    def subsystem_create(self, nqn, sn):
        return self.cli.subsystem_create(nqn, sn)

    def subsystem_create_transport(self, trtype, num_shared_buf):
        return self.cli.subsystem_create_transport(trtype, num_shared_buf)

    def subsystem_list(self):
        return self.cli.subsystem_list()

    def system_set_property(self, impact):
        return self.cli.system_set_property(impact)

    def system_stop(self):
        return self.cli.system_stop()

    def telemetry_start(self):
        return self.cli.telemetry_start()

    def telemetry_stop(self):
        return self.cli.telemetry_stop()

    def volume_create(self, vol_name, vol_size, arr_name, maxiops=0, maxbw=0):
        return self.cli.volume_create(vol_name, vol_size, arr_name, maxiops, maxbw)

    def volume_mount(self, vol_name, subnqn, arr_name):
        return self.cli.volume_mount(vol_name, subnqn, arr_name)


def set_qos(id, pw, ip, cli, dir, array_name, vol_name, limit_type, limit_value=0, min=False):
    try:
        min_limit_value = 10
        prefix = "max"
        if (min is True):
            prefix = "min"
        limit_value = int(limit_value)
        if (limit_type.lower() != "reset" and limit_value < 10 and limit_value != 0):
            lib.printer.red(
                f"Cannot throttle {limit_type} to {limit_value} (minimum_limit_value: {min_limit_value}). Qos Command is ignored")
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
