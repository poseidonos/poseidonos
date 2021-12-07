import json
import lib
import pos


class Initiator:
    def __init__(self, json):
        self.json = json
        self.name = json["NAME"]
        self.id = json["ID"]
        self.pw = json["PW"]
        self.nic_ssh = json["NIC"]["SSH"]
        self.spdk_dir = json["SPDK"]["DIR"]
        self.spdk_tp = json["SPDK"]["TRANSPORT"]
        self.output_dir = json["SPDK"]["DIR"] + "/tmp"
        self.device_list = []
        self.vdbench_dir = json["VDBENCH"]["DIR"]

    def Prepare(self, connect_nvme=False, subsystem_list=[]):
        if -1 == pos.env.remove_directory(self.id, self.pw, self.nic_ssh, self.output_dir):
            return False

        if -1 == pos.env.make_directory(self.id, self.pw, self.nic_ssh, self.output_dir):
            return False

        if connect_nvme is True:
            self.DisconnectNvme(subsystem_list)
            self.ConnectNvme(subsystem_list)

        lib.printer.green(f" '{self.name}' prepared")
        return True

    def Wrapup(self, connect_nvme=False, subsystem_list=[]):
        if connect_nvme is True:
            self.DisconnectNvme(subsystem_list)

        lib.printer.green(f" '{self.name}' wrapped up")
        return True

    def ConnectNvme(self, subsystem_list):
        for subsystem in subsystem_list:
            if self.name == subsystem[0]:
                lib.subproc.sync_run(f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} sudo nohup nvme connect -n {subsystem[1]} -t tcp -a {subsystem[3]} -s {subsystem[4]}")
        device_list = lib.subproc.sync_run(f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} sudo nohup nvme list | awk '/dev/{{print $1}}'").splitlines()
        for device in device_list:
            self.device_list.append(device)

    def DisconnectNvme(self, subsystem_list):
        for subsystem in subsystem_list:
            if self.name == subsystem[0]:
                lib.subproc.sync_run(f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} sudo nohup nvme disconnect -n {subsystem[1]}")

    def GetVolumeIdOfDevice(self, device_list):
        volume_id_list = {}
        for key in device_list:
            cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} sudo nohup nvme list | awk '{{if ($1 == \"{key}\") print $2}}' "
            serial_number = lib.subproc.sync_run(cmd)
            volId = int(serial_number[3:])
            volume_id_list[key] = volId
        return volume_id_list
