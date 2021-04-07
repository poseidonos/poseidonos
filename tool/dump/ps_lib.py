#!/usr/bin/env python3

# I use python 2.7 grammer 
import psutil
import os
import sys
#subprcess.call ('ls -al', shell=True)
def find_ibofos_coredump_and_renaming():
    core_dir_path="/etc/ibofos/core"
    file_list = os.listdir(core_dir_path)
    for path in file_list:
        filename=os.path.basename(path)
        test_dir_from_filename = os.path.dirname(os.path.realpath(filename.replace('!','/') + '/..'))
        test_path_from_script = os.path.dirname(os.path.realpath(__file__+'/../..'))
        print ("test dir from core file name : " + test_dir_from_filename + " test dir in this script : " + test_path_from_script)
        if("!ibofos.core" in filename and test_path_from_script == test_dir_from_filename):
            if(filename != "ibofos.core"):
                command = "mv "+core_dir_path+"/"+path\
                      +" "+core_dir_path+ "/" + "ibofos.core"
                print(command)
                os.system(command)
                print ("\t#### Core Naming is Changed #####")
            else:
                print ("\t#### Naming Change is already done ###")

def kill_wait(flag_kill="nokill"):

    for proc in psutil.process_iter():
        try:
            if "ibofos" in proc.name()[-6:]:
                proc.resume()
                print ("\t### process name : %s ####" % proc.name())
                if (flag_kill == "kill"):
                    print ("\t### Process Kill ####")
                    proc.send_signal(11)
                print ("\t### Process Wait ####")
                proc.wait()

        except psutil.NoSuchProcess:
            pass

    print ("\t#### kill_and_wait.py Script Completely #####")

