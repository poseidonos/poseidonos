import json
import lib
import pos
import time


class Target:
    def __init__(self, json):
        self.json = json
        self.name = json["NAME"]
        self.id = json["ID"]
        self.pw = json["PW"]
        self.nic_ssh = json["NIC"]["SSH"]
        self.nic_ip1 = json["NIC"]["IP1"]
        self.spdk_dir = json["SPDK"]["DIR"]
        self.spdk_tp = json["SPDK"]["TRANSPORT"]["TYPE"]
        self.spdk_no_shd_buf = json["SPDK"]["TRANSPORT"]["NUM_SHARED_BUFFER"]
        self.pos_dir = json["POS"]["DIR"]
        self.pos_bin = json["POS"]["BIN"]
        self.pos_cli = json["POS"]["CLI"]
        self.pos_cfg = json["POS"]["CFG"]
        self.pos_log = json["POS"]["LOG"]

    def Prepare(self):
        # env setting
        result = pos.env.check_pos_running(self.id, self.pw, self.nic_ssh, self.pos_bin)
        if -1 == result:
            return False
        elif result:
            result = pos.env.kill_pos(self.id, self.pw, self.nic_ssh, self.pos_bin)
            if -1 == result:
                return False
            time.sleep(1)
        if -1 == pos.env.copy_pos_config(self.id, self.pw, self.nic_ssh, self.pos_dir, self.pos_cfg):
            return False
        if -1 == pos.env.execute_pos(self.id, self.pw, self.nic_ssh, self.pos_bin, self.pos_dir, self.pos_log):
            return False
        time.sleep(1)

        # spdk setting
        if -1 == pos.cli.transport_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, self.spdk_tp, self.spdk_no_shd_buf):
            return False
        for subsys in self.json["SPDK"]["SUBSYSTEMs"]:
            if -1 == pos.cli.subsystem_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, subsys["NQN"], subsys["SN"]):
                return False
            if -1 == pos.cli.subsystem_add_listener(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, subsys["NQN"],
                                                    self.spdk_tp, self.json["NIC"][subsys["IP"]], subsys["PORT"]):
                return False

        # pos setting
        for array in self.json["POS"]["ARRAYs"]:
            buf_dev = array["BUFFER_DEVICE"]
            if -1 == pos.cli.bdev_malloc_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, buf_dev["NAME"],
                                                buf_dev["TYPE"], buf_dev["NUM_BLOCKS"], buf_dev["BLOCK_SIZE"], buf_dev["NUMA"]):
                return False
        if -1 == pos.cli.device_scan(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir):
            return False
        if -1 == pos.cli.array_reset(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir):
            return False
        for array in self.json["POS"]["ARRAYs"]:
            if -1 == pos.cli.array_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, array["USER_DEVICE_LIST"],
                                          array["SPARE_DEVICE_LIST"], array["NAME"], array["RAID_TYPE"]):
                return False
            if -1 == pos.cli.array_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, array["NAME"]):
                return False
            for volume in array["VOLUMEs"]:
                if -1 == pos.cli.volume_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume["NAME"],
                                               volume["SIZE"], array["NAME"]):
                    return False
                if -1 == pos.cli.volume_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume["NAME"],
                                              volume["SUBNQN"], array["NAME"]):
                    return False

        # print subsystems
        subsys = pos.cli.subsystem_list(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir)
        print(subsys)

        lib.printer.green(f" '{self.name}' prepared")
        return True

    def Wrapup(self):
        for array in self.json["POS"]["ARRAYs"]:
            if -1 == pos.cli.array_unmount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, array["NAME"]):
                return False
        if -1 == pos.cli.system_stop(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir):
            return False
        lib.printer.green(f" '{self.name}' wrapped up")
        return True

    def ForcedExit(self):
        pos.env.kill_pos(self.id, self.pw, self.nic_ssh, self.pos_bin)
        time.sleep(1)
