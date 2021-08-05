import json
import time
import lib
import pos


class Target:
    def __init__(self, json):
        self.json = json
        self.name = json["NAME"]
        self.id = json["ID"]
        self.pw = json["PW"]
        self.nic_ssh = json["NIC"]["SSH"]
        self.nic_ip1 = json["NIC"]["IP1"]
        self.list_cmd = ["sshpass", "-p", self.pw, "ssh", f"{self.id}@{self.nic_ssh}"]
        self.cmd = f"sshpass -p {self.pw} ssh {self.id}@{self.nic_ssh} echo {self.pw} | sudo -s "
        self.spdk_dir = json["SPDK"]["DIR"]
        self.spdk_tp = json["SPDK"]["TRANSPORT"]
        self.spdk_no_shd_buf = json["SPDK"]["NUM_SHARED_BUFFER"]
        self.spdk_write_buf_size_in_mb = json["SPDK"]["WRITE_BUFFER_SIZE_IN_MB"]
        self.pos_dir = json["POS"]["DIR"]
        self.pos_bin = json["POS"]["BIN"]
        self.pos_cli = json["POS"]["CLI"]
        self.pos_cfg = json["POS"]["CFG"]
        self.pos_log = json["POS"]["LOG"]
        self.subsystems = {}

    def Prepare(self):
        # env setting
        result = pos.env.check_pos_running(self.cmd, self.pos_bin)
        if -1 == result:
            return False
        elif result:
            result = pos.env.kill_pos(self.cmd, self.pos_bin)
            if -1 == result:
                return False
            time.sleep(1)
        if -1 == pos.env.copy_pos_config(self.cmd, self.pos_dir, self.pos_cfg):
            return False
        if -1 == pos.env.execute_pos(self.list_cmd, self.pos_bin, self.pos_dir, self.pos_log):
            return False
        time.sleep(1)

        # spdk setting
        if -1 == pos.spdk.nvmf_create_transport(self.cmd, self.spdk_dir, self.spdk_tp, self.spdk_no_shd_buf):
            return False
        if -1 == pos.spdk.bdev_malloc_create(self.cmd, self.spdk_dir, self.spdk_write_buf_size_in_mb):
            return False
        for subsys in self.json["SPDK"]["SUBSYSTEMs"]:
            if -1 == pos.spdk.nvmf_create_subsystem(self.cmd, self.spdk_dir, subsys["NQN"], subsys["SN"]):
                return False
            if -1 == pos.spdk.nvmf_subsystem_add_listener(self.cmd, self.spdk_dir, subsys["NQN"], self.spdk_tp,
                                                          self.json["NIC"][subsys["IP"]], subsys["PORT"]):
                return False

        # pos setting
        if -1 == pos.cli.device_scan(self.cmd, self.pos_cli, self.pos_dir):
            return False
        if -1 == pos.cli.array_reset(self.cmd, self.pos_cli, self.pos_dir):
            return False
        for array in self.json["POS"]["ARRAYs"]:
            if -1 == pos.cli.array_create(self.cmd, self.pos_cli, self.pos_dir, array["USER_DEVICE_LIST"],
                                          array["SPARE_DEVICE_LIST"], array["NAME"], array["RAID_TYPE"]):
                return False
            if -1 == pos.cli.array_mount(self.cmd, self.pos_cli, self.pos_dir, array["NAME"]):
                return False
            for volume in array["VOLUMEs"]:
                if -1 == pos.cli.volume_create(self.cmd, self.pos_cli, self.pos_dir, volume["NAME"],
                                               volume["SIZE"], array["NAME"]):
                    return False
                if -1 == pos.cli.volume_mount(self.cmd, self.pos_cli, self.pos_dir, volume["NAME"],
                                              volume["SUBNQN"], array["NAME"]):
                    return False

        # spdk get subsystems
        str_subsys = pos.spdk.nvmf_get_subsystems(self.cmd, self.spdk_dir)
        self.subsystems = json.loads(str_subsys)

        lib.printer.green(f" '{self.name}' prepared")
        return True

    def Wrapup(self):
        for array in self.json["POS"]["ARRAYs"]:
            if -1 == pos.cli.array_unmount(self.cmd, self.pos_cli, self.pos_dir, array["NAME"]):
                return False
        if -1 == pos.cli.system_stop(self.cmd, self.pos_cli, self.pos_dir):
            return False
        lib.printer.green(f" '{self.name}' wrapped up")
        return True

    def ForcedExit(self):
        pos.env.kill_pos(self.cmd, self.pos_bin)
        time.sleep(1)
