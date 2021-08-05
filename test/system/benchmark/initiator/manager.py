import copy
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
        self.cmd = ["sshpass", f"-p {self.pw} ssh -t -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh}", "sudo"]
        self.fio_cmd = ["sshpass", "-p", self.pw, "ssh", f"{self.id}@{self.nic_ssh}"]
        self.spdk_dir = json["SPDK"]["DIR"]
        self.spdk_tp = json["SPDK"]["TRANSPORT"]
        self.output_dir = json["SPDK"]["DIR"] + "/tmp"

    def Prepare(self):
        try:
            rm_cmd = copy.deepcopy(self.cmd)
            rm_cmd.extend(["rm", "-rf", self.output_dir])
            lib.subproc.popen(rm_cmd, False, False)
        except Exception as e:
            lib.printer.red(rm_cmd)
            lib.printer.red(f"{__name__} [Error] {e}")
            return False

        try:
            mkdir_cmd = copy.deepcopy(self.cmd)
            mkdir_cmd.extend(["mkdir", "-p", self.output_dir])
            lib.subproc.popen(mkdir_cmd, False, False)
        except Exception as e:
            lib.printer.red(mkdir_cmd)
            lib.printer.red(f"{__name__} [Error] {e}")
            return False

        lib.printer.green(f" '{self.name}' prepared")
        return True

    def Wrapup(self):
        lib.printer.green(f" '{self.name}' wrapped up")
        return True
