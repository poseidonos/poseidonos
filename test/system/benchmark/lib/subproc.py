import asyncio
import subprocess

allow_stdout = False


def set_allow_stdout():
    global allow_stdout
    allow_stdout = True


def write_log(cmd):
    if (allow_stdout is True):
        print(cmd)


def sync_run(cmd, ignore_err=False, sh=True):
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=sh
    )
    (out, err) = proc.communicate()
    write_log(cmd)
    err_str = err.decode("utf-8")
    if not ignore_err and 0 < len(err_str):
        raise Exception(err_str)
    return out.decode("utf-8")


async def async_run(cmd, ignore_err=False):
    proc = await asyncio.create_subprocess_shell(
        cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE
    )
    (out, err) = await proc.communicate()
    write_log(cmd)
    err_str = err.decode("utf-8")
    if not ignore_err and 0 < len(err_str):
        raise Exception(err_str)
    return out.decode("utf-8")
