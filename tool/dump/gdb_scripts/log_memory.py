import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def get_in_memory_log():
    logger_list = core_dump_lib.parse_logger_header(
        current_path + '/../../../')
    output_string_list = []
    index = 0
    for logger_module in logger_list:
        output = gdb.execute(
            "p (pos::singletonInfo->logger->dumpModule[%d]->dumpQueue)\n" % (index), to_string=True)
        output_list = output.split('\n')
        output_string_list = output_string_list + output_list
        index = index + 1
    print(len(output_string_list))
    core_dump_lib.internal_parse_dump(
        output_string_list, "poseidonos.inmemory.log")
    print('Result file is \"poseidonos.inmemory.log\"')
