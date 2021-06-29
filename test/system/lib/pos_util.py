#!/usr/bin/env python3

import os, signal
import subprocess
import time

def kill_process(target_process_name):
    print("kill process : " + target_process_name)
    for line in os.popen("ps ax | grep " + target_process_name + " | grep -v grep"):
        fields = line.split() 
        pid = fields[0]
        print("kill process pid : " + str(pid))
        os.kill(int(pid), signal.SIGKILL)
        time.sleep(3)

def abort_process(target_process_name):
    print("abort process : " + target_process_name)
    for line in os.popen("ps ax | grep " + target_process_name + " | grep -v grep"):
        fields = line.split() 
        pid = fields[0]
        print("abort process pid : " + str(pid))
        os.kill(int(pid), signal.SIGABRT)
        time.sleep(3)

def pci_rescan():
    command = ["/bin/bash", "-i" , "-c", "echo 1 > /sys/bus/pci/rescan"]
    subprocess.call(command)
    time.sleep(5)

def pci_detach(dev_name):
    print ("pci_detach : " + dev_name)
    command = ['../../script/detach_device.sh' ,  dev_name , '0']
    subprocess.call(command)
    time.sleep(2)

def pci_detach_and_attach(dev_name):
    command = ['../../script/detach_device.sh' ,  dev_name , '1']
    subprocess.call(command)
    time.sleep(2)