import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")

import gdb_lib
import core_dump_lib


def bdev_information():
    index = 0
    while(1):
        addr = gdb_lib.get_spdk_value("g_nvmf_tgts.tqh_first->subsystems[%d]" % index)
        if (addr == '0x0' or addr == '0'):
            break
        subnqn = gdb_lib.get_spdk_str("g_nvmf_tgts.tqh_first->subsystems[%d]->subnqn" % index)
        print("")
        print("")
        print("######## subsystem id : %d, subsystem nqn : %s #########" % (index, subnqn))
        nsindex = 0
        addr = gdb_lib.get_spdk_value("g_nvmf_tgts.tqh_first->subsystems[%d]->ns" % (index))
        if (addr == '0x0' or addr == '0'):
            index = index + 1
            continue
        while (1):
            addr = gdb_lib.get_spdk_value("g_nvmf_tgts.tqh_first->subsystems[%d]->ns[%d]" % (index, nsindex))
            if (addr == '0x0' or addr == '0'):
                break
            nsid = gdb_lib.get_spdk_value("g_nvmf_tgts.tqh_first->subsystems[%d]->ns[%d].nsid" % (index, nsindex))
            bdev_name = gdb_lib.get_spdk_value("g_nvmf_tgts.tqh_first->subsystems[%d]->ns[%d]->bdev.name" % (index, nsindex))
            print("namespace id : %s, namespace bdev name : %s" % (nsid, bdev_name))
            nsindex = nsindex + 1
        index = index + 1


def pending_volume_io():
    output = gdb.execute("p pos::singletonInfo->arrayManager->arrayList", to_string=True)
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
            pending_read = \
                gdb.parse_and_eval("((ArrayComponents *)%s)->volMgr->volumes->pendingIOCount[%d][0]._M_i" % (array_component, vol))
            pending_write = \
                gdb.parse_and_eval("((ArrayComponents *)%s)->volMgr->volumes->pendingIOCount[%d][1]._M_i" % (array_component, vol))
            pending_internal_io = \
                gdb.parse_and_eval("((ArrayComponents *)%s)->volMgr->volumes->pendingIOCount[%d][2]._M_i" % (array_component, vol))

            print("volume %d read : %s write : %s internal : %s" % (vol, pending_read, pending_write, pending_internal_io))
        index = index + 1
