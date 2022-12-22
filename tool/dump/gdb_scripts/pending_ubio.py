import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def show_pending_ubio():
    output = gdb.execute("p pos::gDumpSharedModulePtr", to_string=True)
    pending_object_list = core_dump_lib.parse_shared_dump_header(
            current_path + '/../../../')
    index = 0
    for pending_object in pending_object_list:
        if(pending_object == 'UBIO'):
            break
        index = index + 1

    memaddr = output.split('=')[1].split(
        ',')[index].rstrip('\n }').lstrip(' {')
    if (memaddr == "0x0" or memaddr == "0"):
        print("Ubio is not used before")
        return
    output = gdb.execute("p ((pos::DumpSharedModule<pos::Ubio *, %d> *) %s)->dumpMap "
                            % (index, memaddr), to_string=True)
    lines = output.split('\n')

    index = 0
    tv_sec = 0.0
    tv_usec = 0.0
    tmp_str = ""
    output_callback_ele = "0x0"
    for line in lines:
        if ("buffer" in line):
            ubio_memaddr = (line.split('=')[1].lstrip(' ').rstrip(' ,\n'))
            output_ubio = gdb.execute(
                "p *((Ubio *)%s)" % (ubio_memaddr), to_string=True).split('\n')
            for line in output_ubio:
                if ("_vptr.DebugInfoInstance" in line):
                    if ("pos::VolumeIo" in line):
                        output_rba = gdb.parse_and_eval(
                            "((VolumeIo *) %s)->sectorRba " % (ubio_memaddr))
                    else:
                        output_rba = "Not VolumeIo"
                    break
            output_dir = gdb.parse_and_eval(
                "((Ubio *) %s)->dir " % (ubio_memaddr))
            output_lba = gdb.parse_and_eval(
                "((Ubio *) %s)->lba " % (ubio_memaddr))
            output_sync = gdb.execute(
                "p ((Ubio *) %s)->sync " % (ubio_memaddr), to_string=True).split('\n')
            output_sync_ele = ""

            for line_sync in output_sync:
                if ("_M_i" in line_sync):
                    output_sync_ele = line_sync.split(
                        '=')[1].rstrip(' \n').lstrip(' ')

            output_callback = (gdb.execute(
                "p ((Ubio *) %s)->callback " % (ubio_memaddr), to_string=True)).split('\n')

            check_callee = False
            output_callback_ele = ""
            for line_callback in output_callback:
                if ("get()" in line_callback):
                    output_callback_ele = line_callback.split(
                        '=')[1].rstrip(' \n').lstrip(' ')
                    if (output_callback_ele == "0x0" or output_callback_ele == "0x0"):
                        break

            tmp_str = ("ubio address : " + ubio_memaddr + " dir : %s rba : %s deviceLba : %s sync : %s callback pointer : %s"
                        % (output_dir, output_rba, output_lba, output_sync_ele, output_callback_ele))

        if ("tv_sec = " in line):
            tv_sec = int(line.split()[2].rstrip(","))
        if ("tv_usec = " in line):
            tv_usec = int(line.split()[2].rstrip(","))
            time_zone = core_dump_lib.check_time_zone("binary_info")
            tmp_str = core_dump_lib.printout_timestamp(
                tv_sec, tv_usec, time_zone) + " " + tmp_str
            print(tmp_str)
            if (output_callback_ele != "" and output_callback_ele != "0x0" and output_callback_ele != "0x0"):
                gdb_lib.show_callback_list(output_callback_ele)
            index = index + 1
            output_callback_ele = "0x0"
            print('')

    if (index == 0):
        print("There are no pending ubio")
