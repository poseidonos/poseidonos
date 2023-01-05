import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")

import gdb_lib
import core_dump_lib


def rba_info(array_id, volume_id, rba_str):
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
    executed = False
    for array_component in array_components:
        if (str(index) != array_id):
            index = index + 1
            continue
        executed = True
        print("===================================")
        print("     Array Name : %s     " % array_name[index])
        print("===================================")

        rba = int(rba_str, 0)
        print("array_id : %s, volume_id : %s, rba (4k unit) : %d" % (array_id, volume_id, rba))

        rbaState = gdb.parse_and_eval("((ArrayComponents *)%s)->rbaStateMgr->rbaStatesInArray._M_elems[%s].rbaStates[%d].ownered._M_i" % (array_component, volume_id, rba))
        print("RbaState : %s" % rbaState)
        print("")
        addr = gdb.parse_and_eval("(void **)&((ArrayComponents *)%s)->meta->mapper->vsaMapManager->vsaMaps[%s]->map.mPageArr[%d].data[%d]" % (array_component, volume_id, rba / 504, (rba % 504) * 8))
        vstripeId = gdb.parse_and_eval("((VirtualBlkAddr *)%s)->stripeId" % addr)
        blkoffset = gdb.parse_and_eval("((VirtualBlkAddr *)%s)->offset" % addr)
        print("vStripe Id : %s, BlockOffset : %s" % (vstripeId, blkoffset))
        addr = gdb.parse_and_eval("(void **)&((ArrayComponents *)%s)->meta->mapper->stripeMapManager->stripeMap->map.mPageArr[%d].data[%d]" % (array_component, vstripeId / 1008, (vstripeId % 1008) * 4))
        stripeLoc = gdb.parse_and_eval("((StripeAddr *)%s)->stripeLoc" % addr)
        lstripeId = gdb.parse_and_eval("((StripeAddr *)%s)->stripeId" % addr)
        print("LStripe Id : %s, stripeLoc : %s" % (lstripeId, stripeLoc))

        output = gdb.execute("p *((ArrayComponents *)%s)" % (array_component), to_string=True)
        output_list = output.split("\n")
        addr = ""
        for output_elem in output_list:
            if ("array = " in output_elem):
                addr = output_elem.split("=")[1].rstrip("\n").rstrip(" ").rstrip(",").lstrip(" ")
                break
        index = 0
        print("Array : %s" % addr)
        for type_index in range(0, 4):
            partition_type = gdb.parse_and_eval("((Array *)%s)->ptnMgr->partitions._M_elems[%d].type" % (addr, type_index))
            if (partition_type == stripeLoc):
                index = type_index
                break

        startLba = gdb.parse_and_eval("((Array *)%s)->ptnMgr->partitions._M_elems[%d].physicalSize.startLba" % (addr, index))
        blksPerChunk = gdb.parse_and_eval("((Array *)%s)->ptnMgr->partitions._M_elems[%d].logicalSize.blksPerChunk" % (addr, index))
        chunksPerStripe = gdb.parse_and_eval("((Array *)%s)->ptnMgr->partitions._M_elems[%d].logicalSize.chunksPerStripe" % (addr, index))

        print("==== Partition Info ====")
        print("stripleLoc : %s, startLba : %s, blksPerChunk : %s, chunksPerStripe : %s" % (stripeLoc, startLba, blksPerChunk, chunksPerStripe))

    if (executed != True):
        print("There are not that array id : %d" % array_id)
