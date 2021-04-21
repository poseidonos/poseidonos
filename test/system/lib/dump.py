import os, signal, sys
import subprocess
import time
import glob
import psutil
import argparse

import cli
import pos_array_api

allowed_nodes = ["pm", "vm"]

def kill_pos_safe(root_dir):
    print("killing poseidonos safe")

    pos_array_api.unmount_all_arrays()
    cli.exit_pos()

    while (check_ibofos_running()):
        time.sleep(1)

def get_meta_size(root_dir):
    command = ["rm", "-f", root_dir + "/output.txt"]
    subprocess.call(command)
    print("deleted output file")

    cli.wbt_request("dump_disk_layout", "")
    
    with open(root_dir + "/output.txt", "r") as f:
        while True:
            line = f.readline()
            if "user data start lba" in line:
                meta_lba = int(line.split()[-1])
                meta_lba = meta_lba * 512 / 4096
                return int(meta_lba * 1.1)

def unbind_ssd(root_dir):
    command = ["make", "udev_uninstall"] 
    subprocess.call(command)
    print("udev_uninstall completed")

    command = [root_dir + "/lib/spdk/scripts/setup.sh", "reset"] 
    subprocess.call(command)
    print("spdk reset completed")

def check_ssd(device):
    actual_devices = []
    not_exist_devices = []
    for filename in os.listdir("/dev"):
        if "nvme" in filename:
            actual_devices.append(filename)

    expected_devices = device.split(",")
    for d in expected_devices:
        if d not in actual_devices:
            not_exist_devices.append(d)

    if not_exist_devices:
        print("%s does not exist in the system. Check device or unbind device manually" % ', '.join(not_exist_devices))
        print("existing devices: %s" % ', '.join(actual_devices))
        sys.exit()
            
    
def dump_ssd(device, count, dump_dir):
    print("SSD dump with count %d" % count)
    # TODO: check whether the dump_directory exists

    for d in device.split(","):
        command = ["dd", "if=/dev/%s" % d, "of=%s/dump_%s.bin" % (dump_dir,d), "bs=4k", "count=%d" % count] 
        subprocess.call(command)
        print("device %s dump completed")

def load_ssd(device, count, dump_dir):
    print("SSD load with count %d" % count)
    # TODO: check whether the dump exists
    
    for d in device.split(","):
        command = ["dd", "of=/dev/%s" % d, "if=%s/dump_%s.bin" % (dump_dir,d), "bs=4k", "count=%d" % count] 
        subprocess.call(command)
        print("device %s load completed")        
        
def check_ibofos_running():
    for proc in psutil.process_iter(['name']):
        try:
            if "poseidonos" == proc.info['name']:
                return True
        except:
            pass
        
    return False

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--node", dest="node", action="store", required=True, type=str, help="node to run this script (pm or vm)")
    parser.add_argument("--dump_directory", dest="dump_directory", action="store", required=True, type=str, help="directory which stores dump files")
    parser.add_argument("--root_directory", dest="root_directory", action="store", required=True, type=str, help="directory of poseidonos")
    parser.add_argument("--device", dest="device", action="store", required=True, type=str, help="comma seperated devices to dump ex) --device nvme0n1,nvme1n1,nvme2n1")
    parser.add_argument("--count", dest="count", action="store", default=0, type=int, help="")
    parser.add_argument("action", metavar="A", type=str, nargs=1, help="dump or load")
    args = parser.parse_args()
    if (args.node not in allowed_nodes):
        sys.exit("node should be pm or vm")
    if not os.path.exists(args.root_directory):
        sys.exit("root_directory does not exist.")
    if not os.path.exists(args.dump_directory):
        os.makedir(args.dump_directory)
    if ("dump" not in args.action) and ("load" not in args.action):
        sys.exit("please specify dump or load ")
    # TODO: Check device exists after unbind         
    return args
    
if __name__ == "__main__":   
    if not os.geteuid() == 0:
        sys.exit("\nOnly root can run this script\n")

    args = parse_arguments()
    
    if (args.count == 0):
        if(check_ibofos_running() is False):
            sys.exit("poseidonos is not running. Please run this script while running poseidonos or give count info")
            # or should I run poseidonos?
        # When poseidonos is running
        count = get_meta_size(args.root_directory)
        kill_pos_safe(args.root_directory)
    else:
        count = args.count
    
    unbind_ssd(args.root_directory)

    check_ssd(args.device)

    if "dump" in args.action:
        dump_ssd(args.device, count, args.dump_directory)
    elif "load" in args.action:
        load_ssd(args.device, count, args.dump_directory)
