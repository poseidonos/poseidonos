#!/usr/bin/env python3

import subprocess, os, sys, psutil

class Nvmf:
    def __init__(self, testname, EXPECT_SUCCESS, SPDK_RPC_PATH):
        self.spdk_rpc_path = SPDK_RPC_PATH

        if EXPECT_SUCCESS == True:
            print (testname + "Success Test ]")
        else: 
            print (testname + "Failed Test ]")

    def create_transport(self, trtype,
            buf_cache_size=None, num_shared_buf=None,   # for tcp 
            io_unit_size_in_bytes=None):    # for rdma
        command = list()
        command += [self.spdk_rpc_path, "nvmf_create_transport", "-t", trtype]

        if 'rdma' == trtype:
            if None != io_unit_size_in_bytes:
                command += ["-u", str(io_unit_size_in_bytes)]

        elif 'tcp' == trtype:
            if None != buf_cache_size and None != num_shared_buf:
                command += ["-b", str(buf_cache_size),
                    "-n", str(num_shared_buf)]
        else:
            return False

        proc = subprocess.Popen(command, stdout=subprocess.PIPE)
        rpc_error = self.check_rpc_error(proc)
        exist = self.check_transport(trtype)
        if rpc_error == False and exist == True:
            return True
        return False

    def create_subsystem(self, nqn, serial_num, model_num, max_ns,
            allow_any_host):
        command = list()
        command.append(self.spdk_rpc_path)
        command += ["nvmf_create_subsystem", "-m", str(max_ns)]
        if True == allow_any_host:
            command.append("-a")
        command += ["-s", serial_num, "-d", model_num, nqn]

        proc = subprocess.Popen(command, stdout=subprocess.PIPE)
        rpc_error = self.check_rpc_error(proc)
        exist = self.check_subsystem(nqn)
        if rpc_error == False and exist == True:
            return True
        return False 

    def delete_subsystem(self, nqn):
        command = [self.spdk_rpc_path, "nvmf_delete_subsystem", nqn]
        proc = subprocess.Popen(command, stdout=subprocess.PIPE)
        rpc_error = self.check_rpc_error(proc)
        exist = self.check_subsystem(nqn)
        if rpc_error == False and exist == False:
            return True
        return False

    def add_subsystem_listener(self, nqn, trtype, traddr, trsvid):
        command = [self.spdk_rpc_path, "nvmf_subsystem_add_listener", nqn,
            "-t", trtype, "-a", traddr, "-s", trsvid]
        proc = subprocess.Popen(command, stdout=subprocess.PIPE)
        rpc_error = self.check_rpc_error(proc)
        if rpc_error == False:
            return True
        return False

    def add_namespace_to_subsystem(self, nqn, bdev_name_to_add,
            namespace_id_to_map):
        command = [self.spdk_rpc_path, "nvmf_subsystem_add_ns", nqn,
            bdev_name_to_add, "-n", str(namespace_id_to_map)]
        proc = subprocess.Popen(command,
                stdout=subprocess.PIPE)
        rpc_error = self.check_rpc_error(proc)
        if rpc_error == False:
            return True
        return False

    def check_rpc_error(self, proc):
        err_check = proc.stdout.read().decode("utf-8").find("error")
        if err_check != -1:
            return True
        return False

    def check_transport(self, trtype):
        transport_info = subprocess.check_output([self.spdk_rpc_path, \
                "nvmf_get_transports"], universal_newlines=True)
        ret = transport_info.find(trtype.upper())
        if ret == -1:
            return False
        return True

    def check_subsystem(self, nqn):
        subsystem_info = subprocess.check_output([self.spdk_rpc_path, \
                "nvmf_get_subsystems"], universal_newlines=True)
        ret = subsystem_info.find(nqn)
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
        subsystem_info = subprocess.Popen([self.spdk_rpc_path, \
            "nvmf_get_subsystems"], stdout = subprocess.PIPE)
        map = {}
        while True:
            subsystem_line = subsystem_info.stdout.readline().decode("utf-8")
            if not subsystem_line:
                break
            if "subsystem" in subsystem_line:
                while True:
                    line = subsystem_info.stdout.readline().decode("utf-8")
                    if not line:
                        break;
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
                    print ("\tMapped Success: " + subsystem + "-" + bdev_name)
                    return True
        print ("\tMapped Failed: " + subsystem + "-" + bdev_name)
        return False 
