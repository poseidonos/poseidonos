import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def show_dump_buffer(input_str):
    output = gdb.execute("p %s->dumpQueue" % (input_str), to_string=True)
    lines = output.split('\n')

    index = 0
    tmp_str = ""
    for line in lines:
        if ("get() = " in line):
            callback_memaddr = (line.split(
                '=')[1].lstrip(' ').rstrip(' ,\n'))
            tmp_str = " dump buffer address : " + callback_memaddr
        if ("tv_sec = " in line):
            tv_sec = int(line.split()[2].rstrip(","))
        if ("tv_usec = " in line):
            tv_usec = int(line.split()[2].rstrip(","))
            time_zone = core_dump_lib.check_time_zone("binary_info")
            tmp_str = core_dump_lib.printout_timestamp(
                tv_sec, tv_usec, time_zone) + " " + tmp_str
            print(tmp_str)
            tmp_str = ""
            index = index + 1
            print('')
    if (index == 0):
        print("There are no log for given dump module")