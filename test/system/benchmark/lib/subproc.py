import asyncio
import subprocess


def sync_run(cmd, ignore_err=False, sh=True):
    proc = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=sh
    )
    (out, err) = proc.communicate()
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
    err_str = err.decode("utf-8")
    if not ignore_err and 0 < len(err_str):
        raise Exception(err_str)
    return out.decode("utf-8")
