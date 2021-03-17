import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib


def show_stqh_first(mem, cnt):

    stqh_first_set = (gdb.execute(
        "p *(struct nvme_request *)%s" % (mem), to_string=True)).split('\n')
    check_callee = False

    for line in stqh_first_set:
        if("cid" in line):
            cid = line.split('=')[1].rstrip(' ,\n').lstrip(' ')
            print(cnt, cid)
            break

    for line in stqh_first_set:
        if("stqe_next" in line):
            stqe_next = line.split('=')[1].rstrip(' ,\n').lstrip(' ')
            if(stqe_next != "0x0"):
                show_stqh_first(stqe_next, cnt + 1)
            break