import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def show_pending_object(object_name):
    output = gdb.execute("p pos::gDumpSharedModulePtr", to_string=True)
    pending_object_list = core_dump_lib.parse_shared_dump_header(
        current_path + '/../../../')
    index = 0
    found_flag = False
    for pending_object in pending_object_list:
        if(pending_object == object_name):
            found_flag = True
            break
        index = index + 1
    if (found_flag is False):
        print("There are no object name ! Please check enum class DumpSharedPtrType!")
        return
    memaddr = output.split('=')[1].split(
        ',')[index].rstrip('\n }').lstrip(' {')
    output = gdb.execute("p ((pos::DumpSharedModule<pos::Ubio *, 0> *) %s)->dumpMap "
                            % (memaddr), to_string=True)
    lines = output.split('\n')

    index = 0
    tmp_str = ""
    for line in lines:
        if ("buffer" in line):
            iocontext_memaddr = (line.split(
                '=')[1].lstrip(' ').rstrip(' ,\n'))
            tmp_str = "dump shared object address : " + iocontext_memaddr
            index = index + 1
        if ("tv_sec = " in line):
            tv_sec = int(line.split()[2].rstrip(","))
        if ("tv_usec = " in line):
            tv_usec = int(line.split()[2].rstrip(","))
            time_zone = core_dump_lib.check_time_zone("binary_info")
            tmp_str = core_dump_lib.printout_timestamp(
                tv_sec, tv_usec, time_zone) + " " + tmp_str
            print(tmp_str)
            tmp_str = ""

    if (index == 0):
        print("There are no pending dump shared object")