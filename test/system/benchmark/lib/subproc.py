import asyncio
import lib
import subprocess

subproc_log = False


def set_print_log(log_option):
    global subproc_log
    subproc_log = log_option


def print_log(cmd):
    if (subproc_log):
        lib.printer.yellow(cmd)


def sync_run(cmd, ignore_err=False, sh=True):
    print_log(cmd)
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=sh
    )
    (out, err) = proc.communicate()
    err_str = err.decode("utf-8")
    if not ignore_err and 0 < len(err_str):
        lib.printer.red(cmd)
        raise Exception(err_str)
    return out.decode("utf-8")


async def async_run(cmd, ignore_err=False):
    print_log(cmd)
    proc = await asyncio.create_subprocess_shell(
        cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE
    )
    (out, err) = await proc.communicate()
    err_str = err.decode("utf-8")
    if not ignore_err and 0 < len(err_str):
        lib.printer.red(cmd)
        raise Exception(err_str)
    return out.decode("utf-8")
