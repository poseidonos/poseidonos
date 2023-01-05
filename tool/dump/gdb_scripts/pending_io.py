import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def show_dev_context(ublock_addr):
    gdb.execute("set print repeats 0", to_string=True)

    output = gdb.execute(
        "p ((pos::UBlockDevice *) " + ublock_addr + ")->property", to_string=True)
    print(output)

    output = gdb.execute(
        "p ((pos::UBlockDevice *) " + ublock_addr + ")->contextArray", to_string=True)

    memaddr = output.split('=')[1].split(',')
    first = True

    index = 0

    for mem in memaddr:
        if (first == True):
            first = False
            continue
        memhex = mem.lstrip(' {').rstrip('\n }')

        if (memhex == '0x0'):
            break

        output = gdb.parse_and_eval(
            '((pos::UnvmeDeviceContext *)' + memhex + ')->pendingIoCount')

        print('\t=== Thread %d\'s Pending Io ===' % (index))
        print('================================================')
        print("pending IO count : ", output)

        output = gdb.parse_and_eval(
            '((pos::UnvmeDeviceContext *)' + memhex + ')->pendingErrorList')

        print("pending Error List : ", output)
        print('================================================')
        index = index + 1

def show_pending_io():

    output = gdb.execute(
        'p pos::singletonInfo->deviceManager->devices', to_string=True)
    output_list = output.split('\n')
    index = 0
    for output_elem in output_list:
        if ("get()" in output_elem):
            memhex = output_elem.split('=')[1].rstrip(',\n ')
            print("###### Device %d Information ###############" % (index))
            show_dev_context(memhex)
            index = index + 1
            print("############################################\n")