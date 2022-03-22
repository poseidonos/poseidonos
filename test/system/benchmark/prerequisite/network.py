import lib


def IrqBalance(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.IrqBalance : {opt}")
    if (not "start" == opt and not "stop" == opt):
        lib.printer.red(f"{__name__}.IrqBalance valid option: start/stop")
        return -1

    try:
        irq_balance = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo systemctl {opt} irqbalance.service"
        lib.subproc.sync_run(irq_balance)
    except Exception as e:
        lib.printer.red(irq_balance)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    return 0


def TcpTune(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.TcpTune : {opt}")
    if (not "min" == opt and not "max" == opt):
        lib.printer.red(f"{__name__}.TcpTune valid option: min/max")
        return -1

    core_mem = "268435456"
    tcp_wmem = "4096 65536 134217728"
    tcp_rmem = "4096 131072 134217728"
    mtu_probing = "1"
    window_scaling = "1"
    slow_start = "0"
    if ("min" == opt):
        core_mem = "212992"
        tcp_wmem = "4096 16384 4194304"
        tcp_rmem = "4096 131072 6291456"
        mtu_probing = "0"
        window_scaling = "0"
        slow_start = "1"

    try:
        rmem_default = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo sysctl -w net.core.rmem_default={core_mem}"
        lib.subproc.sync_run(rmem_default)
    except Exception as e:
        lib.printer.red(rmem_default)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        rmem_max = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo sysctl -w net.core.rmem_max={core_mem}"
        lib.subproc.sync_run(rmem_max)
    except Exception as e:
        lib.printer.red(rmem_max)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        wmem_default = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo sysctl -w net.core.wmem_default={core_mem}"
        lib.subproc.sync_run(wmem_default)
    except Exception as e:
        lib.printer.red(wmem_default)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        wmem_max = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo sysctl -w net.core.wmem_max={core_mem}"
        lib.subproc.sync_run(wmem_max)
    except Exception as e:
        lib.printer.red(wmem_max)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        ipv4_tcp_wmem = ["sshpass", "-p", pw,
                         "ssh", f"{id}@{ip}", "sudo", "nohup"]
        ipv4_tcp_wmem.extend(
            ["sysctl", "-w", f"net.ipv4.tcp_wmem=\"{tcp_wmem}\""])
        lib.subproc.sync_run(ipv4_tcp_wmem, False, False)
    except Exception as e:
        lib.printer.red(ipv4_tcp_wmem)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        ipv4_tcp_rmem = ["sshpass", "-p", pw,
                         "ssh", f"{id}@{ip}", "sudo", "nohup"]
        ipv4_tcp_rmem.extend(
            ["sysctl", "-w", f"net.ipv4.tcp_rmem=\"{tcp_rmem}\""])
        lib.subproc.sync_run(ipv4_tcp_rmem, False, False)
    except Exception as e:
        lib.printer.red(ipv4_tcp_rmem)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        tcp_mtu_probing = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo sysctl -w net.ipv4.tcp_mtu_probing={mtu_probing}"
        lib.subproc.sync_run(tcp_mtu_probing)
    except Exception as e:
        lib.printer.red(tcp_mtu_probing)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        tcp_window_scaling = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo sysctl -w net.ipv4.tcp_window_scaling={window_scaling}"
        lib.subproc.sync_run(tcp_window_scaling)
    except Exception as e:
        lib.printer.red(tcp_window_scaling)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        tcp_slow_start_after_idle = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo sysctl -w net.ipv4.tcp_slow_start_after_idle={slow_start}"
        lib.subproc.sync_run(tcp_slow_start_after_idle)
    except Exception as e:
        lib.printer.red(tcp_slow_start_after_idle)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    return 0


def IrqAffinity(id, pw, ip, opt, pos_dir):
    lib.printer.green(f" + {__name__}.IrqAffinity : {opt}")
    for info in opt:
        nic_node = None
        try:
            get_nic_node = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
                sudo cat /sys/class/net/{info['NIC']}/device/numa_node"
            nic_node = lib.subproc.sync_run(get_nic_node)
            print(f"   {info['NIC']} numa_node : {nic_node}")
        except Exception as e:
            lib.printer.red(get_nic_node)
            lib.printer.red(f"{__name__} [Error] {e}")
            return -1

        try:
            set_irq_aff = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
                sudo {pos_dir}/test/script/set_irq_affinity_cpulist.sh {info['CPU_LIST']} {info['NIC']}"
            lib.subproc.sync_run(set_irq_aff)
        except Exception as e:
            lib.printer.red(set_irq_aff)
            lib.printer.red(f"{__name__} [Error] {e}")
            return -1
    return 0


def Nic(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.Nic : {opt}")
    for info in opt:
        try:
            set_nic = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
                sudo ifconfig {info['INTERFACE']} {info['IP']}/{info['NETMASK']} mtu {info['MTU']}"
            lib.subproc.sync_run(set_nic)
        except Exception as e:
            lib.printer.red(set_nic)
            lib.printer.red(f"{__name__} [Error] {e}")
            return -1
    return 0
