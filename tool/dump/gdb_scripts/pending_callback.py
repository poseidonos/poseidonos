import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def show_pending_callback():
    output = gdb.execute("p pos::gDumpSharedModulePtr", to_string=True)
    pending_object_list = core_dump_lib.parse_shared_dump_header(
        current_path + '/../../../')
    index = 0
    for pending_object in pending_object_list:
        if(pending_object == 'CALLBACK'):
            break
        index = index + 1

    memaddr = output.split('=')[1].split(
        ',')[index].rstrip('\n }').lstrip(' {')
    output = gdb.execute("p ((pos::DumpSharedModule<pos::Callback *, %d> *) %s)->dumpMap "
                            % (index, memaddr), to_string=True)
    lines = output.split('\n')

    index = 0
    tmp_str = ""
    callback_memaddr = "0x0"
    for line in lines:
        if ("buffer" in line):
            callback_memaddr = (line.split(
                '=')[1].lstrip(' ').rstrip(' ,\n'))
            tmp_str = " callback address : " + callback_memaddr
        if ("tv_sec = " in line):
            tv_sec = int(line.split()[2].rstrip(","))
        if ("tv_usec = " in line):
            tv_usec = int(line.split()[2].rstrip(","))
            time_zone = core_dump_lib.check_time_zone("binary_info")
            tmp_str = core_dump_lib.printout_timestamp(
                tv_sec, tv_usec, time_zone) + " " + tmp_str
            print(tmp_str)
            if (callback_memaddr != "0x0" and callback_memaddr != "0"):
                gdb_lib.show_callback_list(callback_memaddr)
            tmp_str = ""
            index = index + 1
            callback_memaddr = "0x0"
            print('')
    if (index == 0):
        print("There are no pending callback")
