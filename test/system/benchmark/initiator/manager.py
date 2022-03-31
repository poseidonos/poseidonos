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

    def Prepare(self, connect_nvme=False, subsystem_list=[]) -> None:
        lib.printer.green(f" {__name__}.Prepare : {self.name}")
        if (self.prereq and self.prereq["CPU"]["RUN"]):
            prerequisite.cpu.Scaling(
                self.id, self.pw, self.nic_ssh, self.prereq["CPU"]["SCALING"])
        if (self.prereq and self.prereq["MEMORY"]["RUN"]):
            prerequisite.memory.MaxMapCount(
                self.id, self.pw, self.nic_ssh, self.prereq["MEMORY"]["MAX_MAP_COUNT"])
            prerequisite.memory.DropCaches(
                self.id, self.pw, self.nic_ssh, self.prereq["MEMORY"]["DROP_CACHES"])
        if (self.prereq and self.prereq["NETWORK"]["RUN"]):
            prerequisite.network.IrqBalance(
                self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["IRQ_BALANCE"])
            if self.prereq["NETWORK"].get("TCP_TUNE"):
                prerequisite.network.TcpTune(
                    self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["TCP_TUNE"])
            prerequisite.network.Nic(
                self.id, self.pw, self.nic_ssh, self.prereq["NETWORK"]["NICs"])
        if (self.prereq and self.prereq["MODPROBE"]["RUN"]):
            prerequisite.modprobe.Modprobe(
                self.id, self.pw, self.nic_ssh, self.prereq["MODPROBE"]["MODs"])
        if (self.prereq and self.prereq["SPDK"]["RUN"]):
            prerequisite.spdk.Setup(
                self.id, self.pw, self.nic_ssh, self.prereq["SPDK"], self.spdk_dir)

        pos.env.remove_directory(
            self.id, self.pw, self.nic_ssh, self.output_dir)
        pos.env.make_directory(self.id, self.pw, self.nic_ssh, self.output_dir)

        if connect_nvme is True:
            self.DisconnectNvme(subsystem_list)
            self.DiscoverNvme(subsystem_list)
            self.ConnectNvme(subsystem_list)

        lib.printer.green(f" '{self.name}' prepared")

    def Wrapup(self, connect_nvme=False, subsystem_list=[]) -> None:
        if connect_nvme is True:
            self.DisconnectNvme(subsystem_list)
        lib.printer.green(f" '{self.name}' wrapped up")

    def DiscoverNvme(self, subsystem_list) -> None:
        for subsystem in subsystem_list:
            if self.name == subsystem[0]:
                nvme_discover_cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                    sudo nvme discover -t {self.spdk_tp} -a {subsystem[3]} -s {subsystem[4]}"
                lib.subproc.sync_run(nvme_discover_cmd)

    def ConnectNvme(self, subsystem_list) -> None:
        for subsystem in subsystem_list:
            if self.name == subsystem[0]:
                nvme_connect_cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                    sudo nvme connect -n {subsystem[1]} -t {self.spdk_tp} -a {subsystem[3]} -s {subsystem[4]}"
                lib.subproc.sync_run(nvme_connect_cmd)
        req_device_list = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
            sudo nvme list"
        device_list = lib.subproc.sync_run(req_device_list)
        device_lines = device_list.split("\n")
        device_num = len(device_lines)
        for device_idx in range(2, device_num - 1):
            for subsystem in subsystem_list:
                if subsystem[2] in device_lines[device_idx]:
                    device_node = device_lines[device_idx].split(" ")[0]
                    self.device_list.append(device_node)
                    break
        print(" KDD Dev List:", self.device_list)

    def DisconnectNvme(self, subsystem_list) -> None:
        for subsystem in subsystem_list:
            if self.name == subsystem[0]:
                nvme_disconnect_cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                    sudo nvme disconnect -n {subsystem[1]}"
                lib.subproc.sync_run(nvme_disconnect_cmd)

    def GetVolumeIdOfDevice(self, device_list):
        volume_id_list = {}
        for key in device_list:
            cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} \
                sudo nvme list | awk '{{if ($1 == \"{key}\") print $2}}'"
            serial_number = lib.subproc.sync_run(cmd)
            volId = int(serial_number[3:])
            volume_id_list[key] = volId
        return volume_id_list
