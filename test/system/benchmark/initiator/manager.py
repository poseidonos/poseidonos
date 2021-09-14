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

    def Prepare(self):
        if -1 == pos.env.remove_directory(self.id, self.pw, self.nic_ssh, self.output_dir):
            return False

        if -1 == pos.env.make_directory(self.id, self.pw, self.nic_ssh, self.output_dir):
            return False

        lib.printer.green(f" '{self.name}' prepared")
        return True

    def Wrapup(self):
        lib.printer.green(f" '{self.name}' wrapped up")
        return True
