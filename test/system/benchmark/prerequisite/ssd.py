import asyncio
import lib


def Format(id, pw, ip, opt, spdk_dir):
    lib.printer.green(f" + {__name__}.Format : {opt}")
    if (not opt):
        return 0
    blk_list = ""

    try:
        rebind_pci_driver = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo {spdk_dir}/scripts/setup.sh reset"
        lib.subproc.sync_run(rebind_pci_driver)
    except Exception as e:
        lib.printer.red(rebind_pci_driver)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    try:
        get_blk_list = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo lsblk -l"
        blk_list = lib.subproc.sync_run(get_blk_list)
    except Exception as e:
        lib.printer.red(get_blk_list)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

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
    nvme_format_cmds = []
    for nvme_disk in nvme_disks:
        nvme_format_cmds.append(f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo nvme format /dev/{nvme_disk} -s 1")
    try:
        asyncio.set_event_loop(asyncio.new_event_loop())
        loop = asyncio.get_event_loop()
        tasks = asyncio.gather(*[
            lib.subproc.async_run(cmd, True) for cmd in nvme_format_cmds
        ])
        loop.run_until_complete(tasks)
        loop.close()
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")

    try:
        bind_pci_driver = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
            sudo {spdk_dir}/scripts/setup.sh"
        lib.subproc.sync_run(bind_pci_driver)
    except Exception as e:
        lib.printer.red(bind_pci_driver)
        lib.printer.red(f"{__name__} [Error] {e}")
        return -1

    return 0
