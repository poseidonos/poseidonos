import json
import lib
import pos
import prerequisite
import time
import os


class Target:
    def __init__(self, json):
        self.json = json
        self.name = json["NAME"]
        self.id = json["ID"]
        self.pw = json["PW"]
        self.nic_ssh = json["NIC"]["SSH"]
        self.nic_ip1 = json["NIC"]["IP1"]
        try:
            self.prereq = json["PREREQUISITE"]
        except Exception as e:
            self.prereq = None
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
        self.array_volume_list = {}

    def Prepare(self):
        lib.printer.green(f" {__name__}.Prepare : {self.name}")
        if (self.prereq and self.prereq["CPU"]["RUN"]):
            prerequisite.cpu.Scaling(self.id, self.pw, self.nic_ssh, self.prereq["CPU"]["SCALING"])
        if (self.prereq and self.prereq["SSD"]["RUN"]):
            prerequisite.ssd.Format(self.id, self.pw, self.nic_ssh, self.prereq["SSD"]["FORMAT"],
                                    self.prereq["SSD"]["UDEV_FILE"], self.spdk_dir, self.pos_dir)
        if (self.prereq and self.prereq["MEMORY"]["RUN"]):
            prerequisite.memory.MaxMapCount(self.id, self.pw, self.nic_ssh, self.prereq["MEMORY"]["MAX_MAP_COUNT"])
            prerequisite.memory.DropCaches(self.id, self.pw, self.nic_ssh, self.prereq["MEMORY"]["DROP_CACHES"])
        if (self.prereq and self.prereq["NETWORK"]["RUN"]):
            prerequisite.network.IrqBalance(self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["IRQ_BALANCE"])
            prerequisite.network.TcpTune(self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["TCP_TUNE"])
            prerequisite.network.IrqAffinity(self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["IRQ_AFFINITYs"],
                                             self.pos_dir)
            prerequisite.network.Nic(self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["NICs"])
        if (self.prereq and self.prereq["MODPROBE"]["RUN"]):
            prerequisite.modprobe.Modprobe(self.id, self.pw, self.nic_ssh, self.prereq["MODPROBE"]["MODs"])
        if (self.prereq and self.prereq["SPDK"]["RUN"]):
            prerequisite.spdk.Setup(self.id, self.pw, self.nic_ssh, self.prereq["SPDK"], self.spdk_dir)
        if (self.prereq and self.prereq["DEBUG"]["RUN"]):
            prerequisite.debug.Ulimit(self.id, self.pw, self.nic_ssh, self.prereq["DEBUG"]["ULIMIT"])
            prerequisite.debug.Apport(self.id, self.pw, self.nic_ssh, self.prereq["DEBUG"]["APPORT"])
            prerequisite.debug.CorePattern(self.id, self.pw, self.nic_ssh, self.prereq["DEBUG"]["DUMP_DIR"],
                                           self.prereq["DEBUG"]["CORE_PATTERN"])

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
        time.sleep(10)

        # spdk setting
        if -1 == pos.cli.transport_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, self.spdk_tp, self.spdk_no_shd_buf):
            return False

        # create subsystem and add listener
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

        pos.cli.telemetry_stop(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir)
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
                    volume_list = []
                    for i in range(vol["NUM"]):
                        nqn = f"nqn.2020-10.pos:subsystem{i+nqn_base+1:02d}"
                        volume_name = f"VOL{i+nqn_base+1}"
                        volume_list.append(volume_name)
                        if -1 == pos.cli.volume_create(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume_name,
                                                       vol["SIZE"], vol["ARRAY"]):
                            return False
                        if -1 == pos.cli.volume_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume_name,
                                                      nqn, vol["ARRAY"]):
                            return False
                    nqn_base += vol["NUM"]
                    self.array_volume_list[vol["ARRAY"]] = volume_list

        pos.cli.logger_setlevel(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, "info")

        # print subsystems
        subsys = pos.cli.subsystem_list(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir)
        print(subsys)

        lib.printer.green(f" '{self.name}' prepared")
        return True

    def CliInLocal(self):
        pos.set_cli_in_local()

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

    def DirtyBringup(self):
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

        # pos setting
        for array in self.json["POS"]["ARRAYs"]:
            if -1 == pos.cli.array_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, array["NAME"]):
                return False
            if "yes" != self.use_autogen:
                for volume in array["VOLUMEs"]:
                    if -1 == pos.cli.volume_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume["NAME"],
                                                  volume["SUBNQN"], array["NAME"]):
                        return False

        # create, mount volume(auto)
        if "yes" == self.use_autogen:
            nqn_base = 0
            for subsys in self.json["AUTO_GENERATE"]["SUBSYSTEMs"]:
                for vol in subsys["VOLUMEs"]:
                    volume_list = []
                    for i in range(vol["NUM"]):
                        nqn = f"nqn.2020-10.pos:subsystem{i+nqn_base+1:02d}"
                        volume_name = f"VOL{i+nqn_base+1}"
                        volume_list.append(volume_name)
                        if -1 == pos.cli.volume_mount(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, volume_name,
                                                      nqn, vol["ARRAY"]):
                            return False
                    nqn_base += vol["NUM"]
                    self.array_volume_list[vol["ARRAY"]] = volume_list

        pos.cli.logger_setlevel(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, "info")

        # print subsystems
        subsys = pos.cli.subsystem_list(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir)
        print(subsys)

        lib.printer.green(f" '{self.name}' prepared")
        return True

    def DetachDevice(self, dev):
        return pos.env.detach_device(self.id, self.pw, self.nic_ssh, dev)

    def PcieScan(self):
        return pos.env.pcie_scan(self.id, self.pw, self.nic_ssh)

    def CheckRebuildComplete(self, arr_name):
        return pos.cli.check_rebuild_complete(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, arr_name)

    def DeviceList(self):
        return pos.cli.device_list(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir)

    def AddSpare(self, arr_name, dev_name):
        return pos.cli.add_spare(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, arr_name, dev_name)

    def SetRebuildImpact(self, impact):
        return pos.cli.set_rebuild_impact(self.id, self.pw, self.nic_ssh, self.pos_cli, self.pos_dir, impact)
