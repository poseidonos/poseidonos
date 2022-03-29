import asyncio
import lib


def Format(id, pw, ip, opt, udev_file, spdk_dir, pos_dir):
    lib.printer.green(f" + {__name__}.Format : {opt}")
    if (not opt):
        return
    blk_list = ""

    # UDD uninstall
    rm_udev_file = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo rm -f {udev_file}"
    lib.subproc.sync_run(rm_udev_file)

    udev_admin = ["sshpass", "-p", pw,
                  "ssh", f"{id}@{ip}", "sudo", "nohup"]
    udev_admin.extend(
        ["udevadm", "control", "--reload-rules", "&&", "udevadm", "trigger"])
    lib.subproc.sync_run(udev_admin, False, False)

    # UDD -> KDD
    rebind_pci_driver = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo {spdk_dir}/scripts/setup.sh reset"
    lib.subproc.sync_run(rebind_pci_driver)

    # Get block device list
    get_blk_list = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo lsblk -l"
    blk_list = lib.subproc.sync_run(get_blk_list)

    # Filter nvme device list except booting disk or partition disk
    blk_lines = blk_list.split("\n")
    blk_num = len(blk_lines)
    nvme_disks = []
    for blk_idx in range(1, blk_num - 1):
        if ("disk" in blk_lines[blk_idx]):
            if ("part" in blk_lines[blk_idx + 1]):
                continue
            elif ("boot" in blk_lines[blk_idx + 1]):
                continue
            elif ("nvme" in blk_lines[blk_idx]):
                nvme_disks.append(blk_lines[blk_idx].split(" ")[0])

    # Format 8 ssds asynchronously, async_run cannot handle more than 30 cmds at once
    nvme_format_cmds = []
    nvme_disk_cnt = 0
    for nvme_disk in nvme_disks:
        nvme_format_cmds.append(f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo nvme format /dev/{nvme_disk} -s 1")
        nvme_disk_cnt += 1
        if (8 == nvme_disk_cnt):
            asyncio.set_event_loop(asyncio.new_event_loop())
            loop = asyncio.get_event_loop()
            tasks = asyncio.gather(*[
                lib.subproc.async_run(cmd, True) for cmd in nvme_format_cmds
            ])
            loop.run_until_complete(tasks)
            loop.close()
            nvme_format_cmds.clear()
            nvme_disk_cnt = 0

    # Get nvme device list to check(display) deivce usage
    nvme_list = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo nvme list"
    print(lib.subproc.sync_run(nvme_list))

    # KDD -> UDD
    bind_pci_driver = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo {spdk_dir}/scripts/setup.sh"
    lib.subproc.sync_run(bind_pci_driver)

    # UDD install
    make_udev_file = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo {pos_dir}/tool/udev/generate_udev_rule.sh"
    lib.subproc.sync_run(make_udev_file)

    cp_udev_file = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo cp {pos_dir}/tool/udev/99-custom-nvme.rules {udev_file}"
    lib.subproc.sync_run(cp_udev_file)

    udev_admin = ["sshpass", "-p", pw,
                  "ssh", f"{id}@{ip}", "sudo", "nohup"]
    udev_admin.extend(
        ["udevadm", "control", "--reload-rules", "&&", "udevadm", "trigger"])
    lib.subproc.sync_run(udev_admin, False, False)
