import copy
import lib
import subprocess


def check_pos_running(id, pw, ip, bin):
    try:
        ps_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo ps -e | grep {bin}"
        result = lib.subproc.sync_run(ps_cmd)
        if -1 == result.find(bin):
            return False
        else:
            return True
    except Exception as e:
        lib.printer.red(ps_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def kill_pos(id, pw, ip, bin):
    try:
        pkill_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo pkill -9 {bin}"
        lib.subproc.sync_run(pkill_cmd)
        return 0
    except Exception as e:
        lib.printer.red(pkill_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def copy_pos_config(id, pw, ip, dir, cfg):
    try:
        copy_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo cp {dir}/config/{cfg} /etc/pos/pos.conf"
        lib.subproc.sync_run(copy_cmd)
        return 0
    except Exception as e:
        lib.printer.red(copy_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def execute_pos(id, pw, ip, bin, dir, log):
    try:
        exe_cmd = ["sshpass", "-p", pw, "ssh", f"{id}@{ip}", "sudo"]
        exe_cmd.extend(["nohup", f"{dir}/bin/{bin}",
                       "&>>", f"{dir}/script/{log}&"])
        lib.subproc.sync_run(exe_cmd, False, False)
    except Exception as e:
        lib.printer.red(exe_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def remove_directory(id, pw, ip, dir):
    try:
        rm_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo rm -rf {dir}"
        lib.subproc.sync_run(rm_cmd)
        return 0
    except Exception as e:
        lib.printer.red(rm_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def make_directory(id, pw, ip, dir):
    try:
        mkdir_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} sudo mkdir -p {dir}"
        lib.subproc.sync_run(mkdir_cmd)
        return 0
    except Exception as e:
        lib.printer.red(mkdir_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1


def detach_device(id, pw, ip, dev):
    try:
        detach_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} 'sudo echo 1 > /sys/bus/pci/devices/{dev}/remove'"
        lib.subproc.sync_run(detach_cmd)
        return True
    except Exception as e:
        lib.printer.red(detach_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return False


def pcie_scan(id, pw, ip):
    try:
        scan_cmd = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} 'sudo echo 1 > /sys/bus/pci/rescan'"
        lib.subproc.sync_run(scan_cmd)
        return True
    except Exception as e:
        lib.printer.red(scan_cmd)
        lib.printer.red(f"{__name__} [Error] {e}")
        return False
