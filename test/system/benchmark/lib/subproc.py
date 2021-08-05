import subprocess


def popen(cmd, ignore_err=False, sh=True):
    recv = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=sh)
    (out, err) = recv.communicate()
    err_str = err.decode("utf-8")
    if not ignore_err and 0 < len(err_str):
        raise Exception(err_str)
    return out.decode("utf-8")
