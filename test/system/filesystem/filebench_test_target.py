#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil

default_fabric_ip = "127.0.0.1"
ibofos_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"

def parse_arguments():
    parser = argparse.ArgumentParser(description='Filebench target')
    parser.add_argument('-f', '--fabric_ip', default=default_fabric_ip,\
            help='Set fabric IP, default: ' + default_fabric_ip)
    global args
    args = parser.parse_args()
    print (args)

def bring_up_ibofos():
    start_ibofos_script = ibofos_root + "test/regression/start_ibofos.sh"
    subprocess.call(start_ibofos_script)
    bring_up_script = ibofos_root + "/test/system/io_path/setup_ibofos_nvmf_volume.sh"
    subprocess.call([bring_up_script,
        "-a", args.fabric_ip,
        "-s", "1",
        "-S", "21474836480",
        "-v", "1",
        "-b", "2048"])

def kill_ibofos():
    for proc in psutil.process_iter():
        try:
            if "ibofos" in proc.name():
                proc.kill()
                proc.wait()
        except psutil.NoSuchProcess:
            pass

if __name__ == "__main__":
    parse_arguments()
    kill_ibofos()
    bring_up_ibofos()
