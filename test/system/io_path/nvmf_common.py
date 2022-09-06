#!/usr/bin/env python3

import subprocess
import os
import sys
import psutil


class Nvmf:
    def __init__(self, testname, EXPECT_SUCCESS, POS_CLI, SPDK_CMD_PATH=""):
        self.spdk_cmd_path = POS_CLI
        self.rpc_cmd = SPDK_CMD_PATH

        if EXPECT_SUCCESS is True:
            print(testname + "Success Test ]")
        else:
            print(testname + "Failed Test ]")

    def create_transport(self, trtype,
                         buf_cache_size=None, num_shared_buf=None,   # for tcp
                         io_unit_size_in_bytes=None):    # for rdma
        command = self.spdk_cmd_path + " subsystem create-transport -t " + trtype

        if trtype is 'tcp':
            if buf_cache_size is not None and num_shared_buf is not None:
                command += " -c " + str(buf_cache_size) +\
                    " --num-shared-buf " + str(num_shared_buf)
        elif trtype is not 'rdma':
            return False

        command += " --json-res | jq '.Response.result.status.code' 2>/dev/null"
        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        out, err = proc.communicate()
        if 0 != int(out.decode("utf-8")):
            print("\tTransport Create Failed")
            return False

        return True

    def create_subsystem(self, stdout_type, nqn, serial_num, model_num, allow_any_host):
        print("\tSubsystem Create")
        command = self.spdk_cmd_path + " subsystem create -q " + nqn
        if True is allow_any_host:
            command += " -o"
        command += " --serial-number " + serial_num + " --model-number " + model_num
        command += " --json-res | jq '.Response.result.status.code' 2>/dev/null"

        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        out, err = proc.communicate()
        if 0 != int(out.decode("utf-8")):
            print("\tSubsystem Create Failed")
            return False

        exist = self.check_subsystem(nqn)
        if exist is True:
            print("\tSubsystem Create Done")
        return exist

    def mount_volume_with_subsystem(self, stdout_type, volume_name, array_name, subnqn, count):
        print("\tMount Volume with Subsystem")
        if not subnqn:
            command = [self.spdk_cmd_path, "volume", "mount", "-v", volume_name, "-a", array_name]
            subprocess.call(command, stdout=stdout_type, stderr=subprocess.STDOUT)
        else:
            command = [self.spdk_cmd_path, "volume", "mount", "-v", volume_name, "-a", array_name,
                       "-q", subnqn, "--force"]
            subprocess.call(command, stdout=stdout_type, stderr=subprocess.STDOUT)
            ret = self.check_subsystem(subnqn)
            if ret is False:
                return False
        print("\tCheck volume mounted")
        ret = self.check_volume_mounted_to_subsystem(volume_name, array_name, subnqn, count)
        print("\tMount Volume with Subsystem Done")
        return ret

    def delete_subsystem(self, stdout_type, nqn):
        print("\tSubsystem Delete")
        command = self.spdk_cmd_path + " subsystem delete -q " + nqn + " --force --json-res | jq '.Response.result.status.code' 2>/dev/null"
        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        out, err = proc.communicate()
        if 0 != int(out.decode("utf-8")):
            print("\tSubsystem Delete Failed")
            return False
        exist = self.check_subsystem(nqn)
        if exist is False:
            print("\tSubsystem Delete Done")
            return True
        return False

    def add_subsystem_listener(self, nqn, trtype, traddr, trsvid):
        command = self.spdk_cmd_path + " subsystem add-listener -q " + nqn +\
            " -t " + trtype + " -i " + traddr + " -p " + trsvid + " --json-res | jq '.Response.result.status.code' 2>/dev/null"
        proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
        out, err = proc.communicate()
        if 0 != int(out.decode("utf-8")):
            print("\tSubsystem Add Listener Failed")
            return False

        return True

    def add_namespace_to_subsystem(self, nqn, bdev_name_to_add,
                                   namespace_id_to_map):
        command = [self.rpc_cmd, "nvmf_subsystem_add_ns", nqn,
                   bdev_name_to_add, "-n", str(namespace_id_to_map)]
        proc = subprocess.Popen(command,
                                stdout=subprocess.PIPE)
        rpc_error = self.check_rpc_error(proc)
        if rpc_error is False:
            return True
        return False

    def check_rpc_error(self, proc):
        err_check = proc.stdout.read().decode("utf-8").find("error")
        if err_check != -1:
            return True
        return False

    def check_transport(self, trtype):
        transport_info = subprocess.check_output([self.rpc_cmd,
                                                  "nvmf_get_transports"], universal_newlines=True)
        ret = transport_info.find(trtype.upper())
        if ret == -1:
            return False
        return True

    def check_listener(self, listener):
        with open(log_path, "r") as file:
            while True:
                line = file.readline().decode("utf-8")
                if not line:
                    break
                if listener in line:
                    return True
        return False

    def get_subsystem_ns_map(self):
        subsystem_info = subprocess.Popen([self.rpc_cmd,
                                           "nvmf_get_subsystems"], stdout=subprocess.PIPE)
        map = {}
        while True:
            subsystem_line = subsystem_info.stdout.readline().decode("utf-8")
            if not subsystem_line:
                break
            if "subsystem" in subsystem_line:
                while True:
                    line = subsystem_info.stdout.readline().decode("utf-8")
                    if not line:
                        break
                    if "bdev_name" in line:
                        map[subsystem_line] = line
                    if "subsystem" in line:
                        subsystem_line = line
                        continue
        return map

    def check_subsystem_ns_mapping(self, map, subsystem, bdev_name):
        for k in map.keys():
            if subsystem in k:
                if bdev_name in map.get(k):
                    print("\tMapped Success: " + subsystem + "-" + bdev_name)
                    return True
        print("\tMapped Failed: " + subsystem + "-" + bdev_name)
        return False

    def check_subsystem(self, subnqn):
        command_line = self.spdk_cmd_path + " subsystem list --json-res | jq -c --arg nqn " + subnqn +\
            " '.Response.result.data.subsystemlist[] | select (.nqn == $nqn)'"
        proc = subprocess.Popen(command_line, shell=True, stdout=subprocess.PIPE)
        out, err = proc.communicate()
        if subnqn not in out.decode("utf-8"):
            print("\tSubsystem Does not Exist: " + subnqn)
            return False
        return True

    def check_volume_mounted_to_subsystem(self, volume_name, array_name, subnqn, count):
        if not subnqn:
            command_line = self.spdk_cmd_path + " volume list -a " + array_name + " --json-res | jq -c --arg vol " +\
                volume_name + " '.Response.result.data.volumes[] | select (.name == $vol) | .status'"
            proc = subprocess.Popen(command_line, shell=True, stdout=subprocess.PIPE)
            out, err = proc.communicate()
            if "Mounted" not in out.decode("utf-8"):
                print("\tVolume(", volume_name, ") failed to Mount on Subsystem")
                return False
        else:
            command_line = self.spdk_cmd_path + " subsystem list --json-res | jq -c --arg nqn " + subnqn +\
                " '.Response.result.data.subsystemlist[] | select (.nqn == $nqn) | .namespaces | length'"
            proc = subprocess.Popen(command_line, shell=True, stdout=subprocess.PIPE)
            out, err = proc.communicate()
            if count != int(out.decode("utf-8")):
                print("\t Volume(", volume_name, ") failed to Mount on Subsystem(", subnqn, ")")
                return False
        return True
