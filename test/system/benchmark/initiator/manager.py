import json
import lib
import pos
import prerequisite


class Initiator:
    def __init__(self, json):
        self.json = json
        self.name = json["NAME"]
        self.id = json["ID"]
        self.pw = json["PW"]
        self.nic_ssh = json["NIC"]["SSH"]
        try:
            self.prereq = json["PREREQUISITE"]
        except Exception as e:
            self.prereq = None
        self.spdk_dir = json["SPDK"]["DIR"]
        self.spdk_tp = json["SPDK"]["TRANSPORT"]
        self.output_dir = json["SPDK"]["DIR"] + "/tmp"
        self.device_list = []
        try:
            self.vdbench_dir = json["VDBENCH"]["DIR"]
        except Exception as e:
            self.vdbench_dir = ""

    def Prepare(self, connect_nvme=False, subsystem_list=[]):
        lib.printer.green(f" {__name__}.Prepare : {self.name}")
        if (self.prereq and self.prereq["CPU"]["RUN"]):
            prerequisite.cpu.Scaling(self.id, self.pw, self.nic_ssh, self.prereq["CPU"]["SCALING"])
        if (self.prereq and self.prereq["MEMORY"]["RUN"]):
            prerequisite.memory.MaxMapCount(self.id, self.pw, self.nic_ssh, self.prereq["MEMORY"]["MAX_MAP_COUNT"])
            prerequisite.memory.DropCaches(self.id, self.pw, self.nic_ssh, self.prereq["MEMORY"]["DROP_CACHES"])
        if (self.prereq and self.prereq["NETWORK"]["RUN"]):
            prerequisite.network.IrqBalance(self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["IRQ_BALANCE"])
            prerequisite.network.TcpTune(self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["TCP_TUNE"])
            prerequisite.network.IrqAffinity(self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["IRQ_AFFINITYs"], self.pos_dir)
            prerequisite.network.Nic(self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["NICs"])
        if (self.prereq and self.prereq["MODPROBE"]["RUN"]):
            prerequisite.modprobe.Modprobe(self.id, self.pw, self.nic_ssh, self.prereq["MODPROBE"]["MODs"])
        if (self.prereq and self.prereq["SPDK"]["RUN"]):
            prerequisite.spdk.Setup(self.id, self.pw, self.nic_ssh, self.prereq["SPDK"], self.pos_dir)

        if -1 == pos.env.remove_directory(self.id, self.pw, self.nic_ssh, self.output_dir):
            return False

        if -1 == pos.env.make_directory(self.id, self.pw, self.nic_ssh, self.output_dir):
            return False

        if connect_nvme is True:
            self.DisconnectNvme(subsystem_list)
            if -1 == self.DiscoverNvme(subsystem_list):
                return False
            if -1 == self.ConnectNvme(subsystem_list):
                return False

        lib.printer.green(f" '{self.name}' prepared")
        return True

    def Wrapup(self, connect_nvme=False, subsystem_list=[]):
        if connect_nvme is True:
            self.DisconnectNvme(subsystem_list)

        lib.printer.green(f" '{self.name}' wrapped up")
        return True

    def DiscoverNvme(self, subsystem_list):
        for subsystem in subsystem_list:
            if self.name == subsystem[0]:
                try:
                    nvme_discover_cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                        sudo nvme discover -t {self.spdk_tp} -a {subsystem[3]} -s {subsystem[4]}"
                    lib.subproc.sync_run(nvme_discover_cmd)
                except Exception as e:
                    lib.printer.red(nvme_discover_cmd)
                    lib.printer.red(f"{__name__} [Error] {e}")
                    return -1
        return 0

    def ConnectNvme(self, subsystem_list):
        for subsystem in subsystem_list:
            if self.name == subsystem[0]:
                try:
                    nvme_connect_cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                        sudo nvme connect -n {subsystem[1]} -t {self.spdk_tp} -a {subsystem[3]} -s {subsystem[4]}"
                    lib.subproc.sync_run(nvme_connect_cmd)
                except Exception as e:
                    lib.printer.red(nvme_connect_cmd)
                    lib.printer.red(f"{__name__} [Error] {e}")
                    return -1
        try:
            req_device_list = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                sudo nvme list"
            device_list = lib.subproc.sync_run(req_device_list)
        except Exception as e:
            lib.printer.red(req_device_list)
            lib.printer.red(f"{__name__} [Error] {e}")
            return -1
        device_lines = device_list.split("\n")
        device_num = len(device_lines)
        for device_idx in range(2, device_num - 1):
            for subsystem in subsystem_list:
                if subsystem[2] in device_lines[device_idx]:
                    device_node = device_lines[device_idx].split(" ")[0]
                    self.device_list.append(device_node)
                    break
        print(" KDD Dev List:", self.device_list)
        return 0

    def DisconnectNvme(self, subsystem_list):
        for subsystem in subsystem_list:
            if self.name == subsystem[0]:
                try:
                    nvme_disconnect_cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                        sudo nvme disconnect -n {subsystem[1]}"
                    lib.subproc.sync_run(nvme_disconnect_cmd)
                except Exception as e:
                    lib.printer.red(nvme_disconnect_cmd)
                    lib.printer.red(f"{__name__} [Error] {e}")
                    return -1
        return 0

    def GetVolumeIdOfDevice(self, device_list):
        volume_id_list = {}
        for key in device_list:
            try:
                cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                    sudo nvme list | awk '{{if ($1 == \"{key}\") print $2}}'"
                serial_number = lib.subproc.sync_run(cmd)
                volId = int(serial_number[3:])
                volume_id_list[key] = volId
            except Exception as e:
                lib.printer.red(cmd)
                lib.printer.red(f"{__name__} [Error] {e}")
                break
        return volume_id_list
