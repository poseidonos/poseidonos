#!/usr/bin/env python3
import os, signal
import subprocess
import time
import glob

def kill_ibofos():
    print("kill pos")
    for line in os.popen("ps ax | grep " + "poseidonos" + " | grep -v grep"):
        fields = line.split() 
        pid = fields[0]
        print("kill process pid : " + str(pid))
        os.kill(int(pid), signal.SIGKILL)
        time.sleep(3)

def config_reset():
    print ("preparing reset config")
    config_file = "../config/pos.conf"
    if os.path.isfile(config_file) == False:
        print("cannot find pos.conf")
        return False
    else:
        command = ["/bin/bash", "-i" , "-c", "cp -f " + config_file + " /etc/pos/"]
        subprocess.call(command)
        print ("config is initialized")
        return True

def mbr_reset():
    print ("preparing reset mbr")
    mbr_reset_script = "../test/script/mbr_reset.sh"
    if os.path.isfile(mbr_reset_script) == False:
        print("cannot find mbr_reset.sh")
        return False
    else:
        subprocess.call([mbr_reset_script])
        print ("mbr is initialized")
        return True

def udev_rule_reset(): 
    print("preparing reset udev rule") 
    path = "../tool/udev/" 
    udev_rule_reset_script = path +"reset_udev_rule.sh" 
    
    if os.path.isfile(udev_rule_reset_script) == False: 
        print("cannot find reset_udev_rule.sh") 
        return False 
    else: 
        subprocess.call([udev_rule_reset_script])
        print("udev rule is initialized")
        return True

def driver_reset(): 
    print("preparing driver reset") 
    setup_env_script = "setup_env.sh" 
   
    if os.path.isfile(setup_env_script) == False: 
        print("cannot find setup_env.sh") 
        return False 
    else: 
        subprocess.call(["./" + setup_env_script]) 
        print ("disk driver is initialized") 
        return True 
   


def clear_log():
    print ("preparing clear logs")
    path='/var/log/pos/'
    for i in glob.glob(os.path.join(path,'*.log')):
        os.remove(i)
    print ("all logs are removed")
    return True

if __name__ == "__main__":
    print ("")
    print (" ================ POS FACTORY RESET ================ ")
    print (" ====================== WARNING ======================= ")
    print (" All information of volume and array including user data will be deleted.")
    print (" - Array and Volume configuration information will be removed")
    print (" - All user data will be removed")
    print (" - All POS logs will be removed")
    print (" - Configuration will be initialized")
    print ("")
    inp = input(" I fully understand the precautions and agree to the initialization.\n The user is responsible for any data loss due to initialization.\n Would you like to continue?(y,n)")

    if inp == "y" or inp == "Y":
        kill_ibofos()
        config_reset()
        time.sleep(1)
        clear_log()
        time.sleep(1)
        mbr_reset()
        time.sleep(1)
        udev_rule_reset()
        time.sleep(1)
        driver_reset()
