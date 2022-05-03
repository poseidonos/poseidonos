#!/usr/bin/python3

import os
import subprocess
import argparse
import psutil
import sys
import time
import threading

current_path = os.path.dirname(os.path.realpath(__file__))
lib_path = os.path.dirname(current_path) + "/lib"
sys.path.insert(1, lib_path)
import remote_procedure

def _remote_execute(ip, id, pw, command, stderr_report=False):
    result = remote_procedure.execute(ip, id, pw, command, stderr_report)
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
