import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def show_debug_info():
    gdb.execute("set print object on", to_string=True)
    output = gdb.execute("p debugInfo", to_string=True)
    lines = output.split('\n')
    for line in lines:
        if '["' in line:
            print ("")
            print ("##############")
            print (line.split('"')[1])
            print ("##############")
            address = line.split()[2].rstrip("\n").rstrip(",")
            output = gdb.execute("p *(DebugInfoInstance *)" + address , to_string=True)
            lines2 = output.split('\n')
            member = False
            for line_elem in lines2:
                if ("Okay" in line_elem):
                    print(line_elem)
                    continue
                if ("members of pos" in line_elem):
                    member = True
                if (member == False):
                    continue
                print(line_elem)

    gdb.execute("set print object off", to_string=True)

def show_debug_info_history():
    f_write = open("debug.info.history", 'w')
    gdb.execute("set print object on", to_string=True)
    output = gdb.execute("p singletonInfo->dumpManager->dumpModules", to_string=True)
    lines = output.split('\n')
    f_write.write("###################\n")
    f_write.write("   Contents        \n")
    f_write.write("###################\n")
    for line in lines:
        if 'History' in line and not 'Error' in line:
            f_write.write(line.split('"')[1] + '\n')
    f_write.write("###################\n")
    f_write.write("\n")
    for line in lines:
        if 'History' in line and not 'Error' in line:
            line_split = False
            if ("Callback_Timeout" in line):
                line_split = True
            f_write.write("\n")
            f_write.write("##############\n")
            f_write.write(line.split('"')[1] + '\n')
            f_write.write("##############\n")
            address = line.split()[2].rstrip("\n").rstrip(",")
            output = gdb.execute("p ((DebugInfoQueueInstance *)" + address + ")->dumpQueue", to_string=True)
            lines2 = output.split('\n')
            tmp_str = ""
            member = False
            for line_elem in lines2:
                if ("date = " in line_elem):
                    member = False
                if (member == True):
                    if ("userSpecificData = 4294954188" in line_elem):
                        tmp_str = tmp_str + " PeriodicHistory "
                    else:
                        tmp_str = tmp_str + line_elem
                    if (line_split == True):
                        tmp_str = tmp_str + '\n'
                if ("members of pos" in line_elem):
                    member = True
                if ("tv_sec = " in line_elem):
                    tv_sec = int(line_elem.split()[2].rstrip(","))
                if ("tv_usec = " in line_elem):
                    tv_usec = int(line_elem.split()[2].rstrip(","))
                    time_zone = core_dump_lib.check_time_zone("binary_info")
                    tmp_str = core_dump_lib.printout_timestamp(
                        tv_sec, tv_usec, time_zone) + " " + tmp_str
                    f_write.write(tmp_str + '\n')
                    member = False
                    tmp_str = ""
    f_write.close()
    print ("Please check file : debug_info.history")

    gdb.execute("set print object off", to_string=True)

    show_debug_info_error_history()

def show_debug_info_error_history():
    f_write = open("debug.info.error.history", 'w')
    gdb.execute("set print object on", to_string=True)
    output = gdb.execute("p singletonInfo->dumpManager->dumpModules", to_string=True)
    lines = output.split('\n')
    f_write.write("###################\n")
    f_write.write("   Contents        \n")
    f_write.write("###################\n")
    for line in lines:
        if 'History' in line and 'Error' in line:
            f_write.write(line.split('"')[1] + '\n')
    f_write.write("###################\n")
    f_write.write("\n")
    for line in lines:
        if 'History' in line and 'Error' in line:
            line_split = False
            if ("Callback_Timeout" in line):
                line_split = True
            f_write.write("\n")
            f_write.write("##############\n")
            f_write.write(line.split('"')[1] + '\n')
            f_write.write("##############\n")
            address = line.split()[2].rstrip("\n").rstrip(",")
            output = gdb.execute("p ((DebugInfoQueueInstance *)" + address + ")->dumpQueue", to_string=True)
            lines2 = output.split('\n')
            tmp_str = ""
            member = False
            for line_elem in lines2:
                if ("date = " in line_elem):
                    member = False
                if (member == True):
                    if ("userSpecificData = 4294954188" in line_elem):
                        tmp_str = tmp_str + " PeriodicHistory "
                    else:
                        tmp_str = tmp_str + line_elem
                    if (line_split == True):
                        tmp_str = tmp_str + '\n'
                if ("members of pos" in line_elem):
                    member = True
                if ("tv_sec = " in line_elem):
                    tv_sec = int(line_elem.split()[2].rstrip(","))
                if ("tv_usec = " in line_elem):
                    tv_usec = int(line_elem.split()[2].rstrip(","))
                    time_zone = core_dump_lib.check_time_zone("binary_info")
                    tmp_str = core_dump_lib.printout_timestamp(
                        tv_sec, tv_usec, time_zone) + " " + tmp_str
                    f_write.write(tmp_str + '\n')
                    member = False
                    tmp_str = ""
    f_write.close()
    print ("Please check file : debug_info.history")

    gdb.execute("set print object off", to_string=True)