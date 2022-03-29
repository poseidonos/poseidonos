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
        self.pos_cfg = json["POS"]["CFG"]
        self.pos_log = json["POS"]["LOG"]
        self.use_autogen = json["AUTO_GENERATE"]["USE"]
        self.subsystem_list = []
        self.array_volume_list = {}
        self.volume_size = ""
        self.cli = pos.cli.Cli(json)

    def Prepare(self) -> None:
        lib.printer.green(f" {__name__}.Prepare : {self.name}")

        if (self.prereq and self.prereq["CPU"]["RUN"]):
            prerequisite.cpu.Scaling(
                self.id, self.pw, self.nic_ssh, self.prereq["CPU"]["SCALING"])
        if (self.prereq and self.prereq["SSD"]["RUN"]):
            prerequisite.ssd.Format(self.id, self.pw, self.nic_ssh, self.prereq["SSD"]["FORMAT"],
                                    self.prereq["SSD"]["UDEV_FILE"], self.spdk_dir, self.pos_dir)
        if (self.prereq and self.prereq["MEMORY"]["RUN"]):
            prerequisite.memory.MaxMapCount(
                self.id, self.pw, self.nic_ssh, self.prereq["MEMORY"]["MAX_MAP_COUNT"])
            prerequisite.memory.DropCaches(
                self.id, self.pw, self.nic_ssh, self.prereq["MEMORY"]["DROP_CACHES"])
        if (self.prereq and self.prereq["NETWORK"]["RUN"]):
            prerequisite.network.IrqBalance(
                self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["IRQ_BALANCE"])
            prerequisite.network.TcpTune(
                self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["TCP_TUNE"])
            prerequisite.network.IrqAffinity(self.id, self.pw, self.nic_ssh,
                                             self.prereq["NETWORK"]["IRQ_AFFINITYs"],
                                             self.pos_dir)
            prerequisite.network.Nic(
                self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["NICs"])
        if (self.prereq and self.prereq["MODPROBE"]["RUN"]):
            prerequisite.modprobe.Modprobe(
                self.id, self.pw, self.nic_ssh, self.prereq["MODPROBE"]["MODs"])
        if (self.prereq and self.prereq["SPDK"]["RUN"]):
            prerequisite.spdk.Setup(
                self.id, self.pw, self.nic_ssh, self.prereq["SPDK"], self.spdk_dir)
        if (self.prereq and self.prereq["DEBUG"]["RUN"]):
            prerequisite.debug.Ulimit(
                self.id, self.pw, self.nic_ssh, self.prereq["DEBUG"]["ULIMIT"])
            prerequisite.debug.Apport(
                self.id, self.pw, self.nic_ssh, self.prereq["DEBUG"]["APPORT"])
            prerequisite.debug.CorePattern(self.id, self.pw, self.nic_ssh,
                                           self.prereq["DEBUG"]["DUMP_DIR"],
                                           self.prereq["DEBUG"]["CORE_PATTERN"])

        result = pos.env.is_pos_running(
            self.id, self.pw, self.nic_ssh, self.pos_bin)
        if (result):
            pos.env.kill_pos(self.id, self.pw, self.nic_ssh, self.pos_bin)
            time.sleep(1)
        pos.env.copy_pos_config(
            self.id, self.pw, self.nic_ssh, self.pos_dir, self.pos_cfg)
        pos.env.execute_pos(self.id, self.pw, self.nic_ssh,
                            self.pos_bin, self.pos_dir, self.pos_log)
        time.sleep(3)

        # spdk setting
        self.cli.subsystem_create_transport(self.spdk_tp, self.spdk_no_shd_buf)

        # create subsystem and add listener
        if "yes" == self.use_autogen:
            nqn_base = 0
            for subsys in self.json["AUTO_GENERATE"]["SUBSYSTEMs"]:
                for i in range(subsys["NUM"]):
                    nqn = f"nqn.2020-10.pos\\:subsystem{i+nqn_base+1:02d}"
                    sn = f"POS000000000000{i+nqn_base+1:02d}"
                    self.cli.subsystem_create(nqn, sn)
                    self.cli.subsystem_add_listener(
                        nqn, self.spdk_tp, self.json["NIC"][subsys["IP"]], subsys["PORT"])
                    subsystem_tmp = [subsys["INITIATOR"], nqn, sn,
                                     self.json["NIC"][subsys["IP"]], subsys["PORT"]]
                    self.subsystem_list.append(subsystem_tmp)
                nqn_base += subsys["NUM"]
        else:
            for subsys in self.json["SPDK"]["SUBSYSTEMs"]:
                self.cli.subsystem_create(subsys["NQN"], subsys["SN"])
                self.cli.subsystem_add_listener(
                    subsys["NQN"], self.spdk_tp, self.json["NIC"][subsys["IP"]], subsys["PORT"])

        # telemetry start
        time.sleep(10)
        self.cli.telemetry_start()

        # pos setting
        for array in self.json["POS"]["ARRAYs"]:
            buf_dev = array["BUFFER_DEVICE"]
            self.cli.device_create(
                buf_dev["NAME"], buf_dev["TYPE"], buf_dev["NUM_BLOCKS"], buf_dev["BLOCK_SIZE"], buf_dev["NUMA"])

        self.cli.device_scan()
        self.cli.array_reset()
        for array in self.json["POS"]["ARRAYs"]:
            self.cli.array_create(array["BUFFER_DEVICE"]["NAME"], array["USER_DEVICE_LIST"],
                                  array["SPARE_DEVICE_LIST"], array["NAME"], array["RAID_TYPE"])
            self.cli.array_mount(array["NAME"])
            if "yes" != self.use_autogen:
                for volume in array["VOLUMEs"]:
                    self.cli.volume_create(
                        volume["NAME"], volume["SIZE"], array["NAME"])
                    self.cli.volume_mount(
                        volume["NAME"], volume["SUBNQN"], array["NAME"])

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
                        self.volume_size = vol["SIZE"]
                        self.cli.volume_create(
                            volume_name, vol["SIZE"], vol["ARRAY"])
                        self.cli.volume_mount(volume_name, nqn, vol["ARRAY"])
                    nqn_base += vol["NUM"]
                    self.array_volume_list[vol["ARRAY"]] = volume_list

        self.cli.logger_set_level("info")

        # print subsystems
        print(self.cli.subsystem_list())

        lib.printer.green(f" '{self.name}' prepared")

    def Wrapup(self) -> None:
        for array in self.json["POS"]["ARRAYs"]:
            self.cli.array_unmount(array["NAME"])
        self.cli.system_stop()
        lib.printer.green(f" '{self.name}' wrapped up")

    def ForcedExit(self) -> None:
        pos.env.kill_pos(self.id, self.pw, self.nic_ssh, self.pos_bin)
        lib.printer.green(f" '{self.name}' forced exited")
        time.sleep(1)

    def DirtyBringup(self) -> None:
        pos.env.copy_pos_config(
            self.id, self.pw, self.nic_ssh, self.pos_dir, self.pos_cfg)
        pos.env.execute_pos(self.id, self.pw, self.nic_ssh,
                            self.pos_bin, self.pos_dir, self.pos_log)
        time.sleep(1)

        # spdk setting
        self.cli.subsystem_create_transport(self.spdk_tp, self.spdk_no_shd_buf)

        if "yes" == self.use_autogen:
            nqn_base = 0
            for subsys in self.json["AUTO_GENERATE"]["SUBSYSTEMs"]:
                for i in range(subsys["NUM"]):
                    nqn = f"nqn.2020-10.pos\\:subsystem{i+nqn_base+1:02d}"
                    sn = f"POS000000000000{i+nqn_base+1:02d}"
                    self.cli.subsystem_create(nqn, sn)
                    self.cli.subsystem_add_listener(
                        nqn, self.spdk_tp, self.json["NIC"][subsys["IP"]], subsys["PORT"])
                    subsystem_tmp = [subsys["INITIATOR"], nqn, sn,
                                     self.json["NIC"][subsys["IP"]], subsys["PORT"]]
                    self.subsystem_list.append(subsystem_tmp)
                nqn_base += subsys["NUM"]
        else:
            for subsys in self.json["SPDK"]["SUBSYSTEMs"]:
                self.cli.subsystem_create(subsys["NQN"], subsys["SN"])
                self.cli.subsystem_add_listener(
                    subsys["NQN"], self.spdk_tp, self.json["NIC"][subsys["IP"]], subsys["PORT"])

        # pos setting
        for array in self.json["POS"]["ARRAYs"]:
            buf_dev = array["BUFFER_DEVICE"]
            self.cli.device_create(
                buf_dev["NAME"], buf_dev["TYPE"], buf_dev["NUM_BLOCKS"], buf_dev["BLOCK_SIZE"], buf_dev["NUMA"])
        self.cli.device_scan()

        # pos setting
        for array in self.json["POS"]["ARRAYs"]:
            self.cli.array_mount(array["NAME"])
            if "yes" != self.use_autogen:
                for volume in array["VOLUMEs"]:
                    self.cli.volume_mount(
                        volume["NAME"], volume["SUBNQN"], array["NAME"])

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
                        self.cli.volume_mount(volume_name, nqn, vol["ARRAY"])
                    nqn_base += vol["NUM"]
                    self.array_volume_list[vol["ARRAY"]] = volume_list

        self.cli.logger_set_level("info")

        # print subsystems
        subsys = self.cli.subsystem_list()
        print(subsys)

        lib.printer.green(f" '{self.name}' prepared")

    def DetachDevice(self, dev) -> None:
        return pos.env.detach_device(self.id, self.pw, self.nic_ssh, dev)

    def PcieScan(self) -> None:
        return pos.env.pcie_scan(self.id, self.pw, self.nic_ssh)

    def CheckRebuildComplete(self, arr_name):
        return self.cli.array_list(arr_name)

    def DeviceList(self):
        return self.cli.device_list()

    def AddSpare(self, arr_name, dev_name):
        return self.cli.array_add_spare(arr_name, dev_name)

    def SetRebuildImpact(self, impact):
        return self.cli.system_set_property(impact)
