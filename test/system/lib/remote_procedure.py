#!/usr/bin/python3

import paramiko


def execute(ip, id, pw, command, stderr_report=False):
    cli = paramiko.SSHClient()
    cli.load_system_host_keys()
    cli.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    cli.connect(ip, port=22, username=id, password=pw)
    stdin, stdout, stderr = cli.exec_command(command)
    result = ""

    exit_status = stdout.channel.recv_exit_status()
    stdout.channel.close()

    if (stderr_report is True):
        stderr.channel.close()
        for line in iter(stderr.readline, ""):
            print(line, end="")
            result += line

    for line in iter(stdout.readline, ""):
        print(line, end="")
        result += line

    cli.close()

    if (exit_status is not 0):
        raise Exception(ip, command)

    return result
