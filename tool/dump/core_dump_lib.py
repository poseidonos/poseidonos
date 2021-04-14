#!/usr/bin/env python3

from datetime import datetime
import os
import pytz

#subprcess.call ('ls -al', shell=True)
GDB_SCRIPT     = 'parse_debug_dumpmanager_trace.gdb'
GDB_SCRIPT_OUT = 'parse_debug_dumpmanager_trace.gdb.out'
LOGGER_HEADER  = 'src/logger/logger.h'
DUMP_HEADER    = 'src/dump/dump_shared_ptr.h'

def parse_logger_header(ibofos_root):
    logger_list = []
    f = open(ibofos_root + LOGGER_HEADER)
    in_class = False       
    for line in f:
        if ("enum class ModuleInDebugLogDump" in line):
            in_class = True;

        if(in_class == True):
        
            if("MAX_SIZE" in line):
                break;

            if(","  in line):
                elem = line.split(",")
                for e in elem:
                    st = e.lstrip(" ").rstrip(" ").rstrip("\n")
                    if(st != ""):
                        logger_list.append(st)

    f.close()
    return logger_list


def parse_shared_dump_header(ibofos_root):
    shared_dump_list = []
    f = open(ibofos_root + DUMP_HEADER)
    in_class = False       
    for line in f:
        if ("enum class DumpSharedPtrType" in line):
            in_class = True;

        if(in_class == True):
        
            if("MAX_DUMP_PTR" in line):
                break;

            if(","  in line):
                elem = line.split(",")
                for e in elem:
                    st = e.lstrip(" ").rstrip(" ").rstrip("\n")
                    if(st != ""):
                        shared_dump_list.append(st)

    f.close() 
    return shared_dump_list



def check_module_number_and_change_script():

    logger_list = parse_logger_header('../../')
    print (logger_list)
    max_count = len(logger_list)

    f = open(GDB_SCRIPT)
    f_write = open(GDB_SCRIPT_OUT, 'w')
    for line in f:
        if ("ReplaceThisStatement" in line):
            for index in range(0, max_count):
                f_write.write("p (pos::debugInfo->logger->dumpModule[%d]->dumpQueue)\n" % (index))
        else:
            f_write.write(line)

    f.close()

def check_time_zone(binary_info_file):
    st = "Asia/Seoul"
    try:
        f = open (binary_info_file)        
        for line in f:
            if('Time zone:' in line):
                st = line.split()[2]
                return st
        return st
    except:
        return st


def internal_parse_dump(gdb_input_str_vector, gdb_output_file):
    current_path = os.path.dirname(os.path.realpath(__file__))
    logger_list = parse_logger_header(current_path + '/../../')
    print (logger_list)
    f_write = open(gdb_output_file, 'w')
    f_out_str = {}
    laststr = ""
    printoutCount = 0
    module_index = -1

    time_zone = check_time_zone("binary_info")
    first_number = -1
    for line in gdb_input_str_vector:
        if ( "$" in line and "=" in line):
            if (first_number == -1):
                first_number = int(line.split("$")[1].split(" ")[0])
                module_index = 0
            else:
                module_index = int(line.split("$")[1].split(" ")[0])-first_number
        if ( "get() = " in line):
            laststr = line.split("\"")[1]
            printoutCount = printoutCount + 1
        if ( "tv_sec = " in line):
            tv_sec = int(line.split()[2].rstrip(","))
            printoutCount = printoutCount + 1
        if ( "tv_usec = " in line):
            tv_usec = int(line.split()[2].rstrip(","))
            printoutCount = printoutCount + 1
        if (printoutCount == 3):
            date = datetime.fromtimestamp(tv_sec, pytz.timezone(time_zone))
            absolute_timestamp = tv_sec * 1000000 + tv_usec
            fmt = '%Y-%m-%d %H:%M:%S'
            f_out_str[absolute_timestamp] = (str(date.strftime(fmt))+ "."+\
                ("%06ld" % tv_usec) + ("[%s]" % (logger_list[module_index])) + laststr + '\n')
            printoutCount = 0

    for key, value in sorted(f_out_str.items()):
        f_write.write(value)
    f_write.close()

def parse_dump(gdb_input_file, gdb_output_file):
    f = open(gdb_input_file)
    internal_parse_dump(f, gdb_output_file)
    f.close()



