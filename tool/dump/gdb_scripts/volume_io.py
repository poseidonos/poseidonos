import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def bdev_information():
    gdb_lib.switch_to_c_stack()
    tmp_str = "g_pos_disks->tqh_first"
    while(1):
        addr = gdb.parse_and_eval(tmp_str)
        if (addr == "0" or addr == "0x0"):
            break
        val = gdb.parse_and_eval(tmp_str + "->volume->name")
        print(val)
        tmp_str = tmp_str + "->link.tqe_next"


def pending_volume_io():
    output = gdb.execute("p pos::debugInfo->arrayManager->arrayList", to_string=True)
    output_list = output.split('\n')
    array_components = []
    array_name = []
    for output_elem in output_list:
        if ("[" in output_elem and "=" in output_elem):
            print(output_elem)
            print(output_elem.split('"')[1])
            array_name.append(output_elem.split('"')[1])
            array_components.append(output_elem.split('=')[1].lstrip(' ').rstrip(' ,\n'))

    index = 0
    for array_component in array_components:
        print("===================================")
        print("     Array Name : %s     " % array_name[index])
        print("===================================")
        for vol in range(0, 256):
            pending_unmounted_io = \
                gdb.parse_and_eval("((ArrayComponents *)%s)->volMgr->volumes->pendingIOCount[%d][0]._M_i" % (array_component, vol))
            pending_mounted_io = \
                gdb.parse_and_eval("((ArrayComponents *)%s)->volMgr->volumes->pendingIOCount[%d][1]._M_i" % (array_component, vol))

            print("volume %d pendingGc : %s pendingHostIo : %s" % (vol, pending_unmounted_io, pending_mounted_io))
        index = index + 1


def volume_io():
    pending_volume_io()
    bdev_information()
