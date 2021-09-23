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
        self.spdk_dir = json["DIR"] + "/lib/spdk"
        self.spdk_tp = json["SPDK"]["TRANSPORT"]["TYPE"]
        self.spdk_no_shd_buf = json["SPDK"]["TRANSPORT"]["NUM_SHARED_BUFFER"]
        self.pos_dir = json["DIR"]
        self.pos_bin = json["POS"]["BIN"]
        self.pos_cli = json["POS"]["CLI"]
        self.pos_cfg = json["POS"]["CFG"]
        self.pos_log = json["POS"]["LOG"]
        self.use_autogen = json["AUTO_GENERATE"]["USE"]
        self.subsystem_list = []

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

        if "yes" == self.use_autogen:
            nqn_base = 0
            for subsys in self.json["AUTO_GENERATE"]["SUBSYSTEMs"]:
                for i in range(subsys["NUM"]):
                    nqn = f"nqn.2020-10.pos\\:subsystem{i+nqn_base+1:02d}"
                    sn = f"POS000000000000{i+nqn_base+1:02d}"
                    if -1 == pos.cli.subsystem_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, nqn, sn):
                        return False
                    if -1 == pos.cli.subsystem_add_listener(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, nqn,
                                                            self.spdk_tp, self.json["NIC"][subsys["IP"]], subsys["PORT"]):
                        return False
                    subsystem_tmp = [subsys["INITIATOR"], nqn, sn, self.json["NIC"][subsys["IP"]], subsys["PORT"]]
                    self.subsystem_list.append(subsystem_tmp)
                nqn_base += subsys["NUM"]

        else:
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
            if -1 == pos.cli.array_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, array["BUFFER_DEVICE"]["NAME"], array["USER_DEVICE_LIST"],
                                          array["SPARE_DEVICE_LIST"], array["NAME"], array["RAID_TYPE"]):
                return False
            if -1 == pos.cli.array_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, array["NAME"]):
                return False
            if "yes" != self.use_autogen:
                for volume in array["VOLUMEs"]:
                    if -1 == pos.cli.volume_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume["NAME"],
                                                   volume["SIZE"], array["NAME"]):
                        return False
                    if -1 == pos.cli.volume_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume["NAME"],
                                                  volume["SUBNQN"], array["NAME"]):
                        return False

        # create, mount volume(auto)
        if "yes" == self.use_autogen:
            nqn_base = 0
            for subsys in self.json["AUTO_GENERATE"]["SUBSYSTEMs"]:
                for vol in subsys["VOLUMEs"]:
                    for i in range(vol["NUM"]):
                        nqn = f"nqn.2020-10.pos:subsystem{i+nqn_base+1:02d}"
                        volume_name = f"VOL{i+nqn_base+1}"
                        if -1 == pos.cli.volume_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume_name,
                                                       vol["SIZE"], vol["ARRAY"]):
                            return False
                        if -1 == pos.cli.volume_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume_name,
                                                      nqn, vol["ARRAY"]):
                            return False
                    nqn_base += vol["NUM"]

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
