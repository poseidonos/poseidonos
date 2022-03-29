import lib


def MaxMapCount(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.MaxMapCount : {opt}")
    if (not isinstance(opt, int) or 0 > opt):
        lib.printer.red(
            f"{__name__}.MaxMapCount valid option: Unsigned-int type value")
        return
    set_max_map_count = f"sshpass -p {pw} ssh -o StrictHostKeyChecking=no {id}@{ip} \
        sudo sysctl -w vm.max_map_count={opt}"
    lib.subproc.sync_run(set_max_map_count)


def DropCaches(id, pw, ip, opt):
    lib.printer.green(f" + {__name__}.DropCaches : {opt}")
    if (not isinstance(opt, int) or 1 > opt or 3 < opt):
        lib.printer.red(
            f"{__name__}.DropCaches valid option: 1(Clear PageCache only), 2(Clear dentries & inodes), 3(Clear all)")
        return
    drop_caches = ["sshpass", "-p", pw,
                   "ssh", f"{id}@{ip}", "sudo", "nohup"]
    drop_caches.extend(["echo", f"{opt}", ">", "/proc/sys/vm/drop_caches"])
    lib.subproc.sync_run(drop_caches, False, False)
