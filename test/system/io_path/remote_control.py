#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil
import sys
import paramiko
import time
import threading


def _remote_execute(ip, id, pw, command, stderr_report=False):
    cli = paramiko.SSHClient()
    cli.load_system_host_keys()
    cli.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    cli.connect(ip, port=22, username=id, password=pw)
    stdin, stdout, stderr = cli.exec_command(command)
    result = ""
    if (stderr_report is True):
        for line in iter(stderr.readline, ""):
            print(line, end="")
            result += line
    for line in iter(stdout.readline, ""):
        print(line, end="")
        result += line
    cli.close()
    return result


class Worker(threading.Thread):
    def __init__(self, name):
        super().__init__()
        self.name = name

    def run(self):
        global thread_param_ip, thread_param_id, thread_param_pw, thread_param_command
        print("sub thread start ", threading.currentThread().getName())
        remote_execute(thread_param_ip, thread_param_id, thread_param_pw, thread_param_command)
        print("sub thread end ", threading.currentThread().getName())


def remote_execute(ip, id, pw, command, stderr_report=False, asyncflag=False):
    if (asyncflag is False):
        return _remote_execute(ip, id, pw, command, stderr_report)
    else:
        global thread_param_ip, thread_param_id, thread_param_pw, thread_param_command
        thread_param_ip = ip
        thread_param_id = id
        thread_param_pw = pw
        thread_param_command = command
        t = Worker("async thread")
        t.start()
        return t
