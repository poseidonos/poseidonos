import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib


def get_spdk_value(inp_str):
    tmp_str = gdb.execute("p " + inp_str, to_string=True)
    return tmp_str.split(" ")[-1].rstrip("\n")


def get_spdk_str(inp_str):
    tmp_str = gdb.execute("p " + inp_str, to_string=True)
    return tmp_str.split("\"")[1]


def show_callback_list(callback_mem):

    callback_set = (gdb.execute("p *((Callback *) %s) " %
                    (callback_mem), to_string=True)).split('\n')
    check_callee = False
    output_callee = ""
    output_subclass = ""
    for line_callback_set in callback_set:
        if("vptr.Event" in line_callback_set):
            output_subclass = line_callback_set.split(
                'for')[1].split('+')[0].rstrip(' \n').lstrip(' ')
        if("callee" in line_callback_set):
            check_callee = True

        if(check_callee is True and "get()" in line_callback_set):
            output_callee = line_callback_set.split(
                '=')[1].rstrip(' \n').lstrip(' ')
            check_callee = False

    print("callback ptr : %s | callback subclass : %s | callback's callee : %s" % (
        callback_mem, output_subclass, output_callee))
    if (output_callee != "" and output_callee != "0x0" and output_callee != "0"):
        show_callback_list(output_callee)


def check_log_level_debug():
    output = str(gdb.parse_and_eval(
        "pos::singletonInfo->logger->preferences->logLevel"))
    print(output)
    if ("spdlog::level::debug" in output):
        return True
    else:
        return False


def switch_to_pos_stack():
    print("Add Poseidonos stack")
    output = gdb.execute("thread apply all bt", to_string=True)
    thread_num = -1
    callstack_num = 0
    line_list = output.split('\n')
    for line in line_list:
        if ("Thread" in line[0:8]):
            split_str = line.split()
            if (len(split_str) >= 2):
                thread_num = int(split_str[1])
        if ("in pos::" in line):
            split_str = line.split()
            callstack_num = int(split_str[0].lstrip("#"))
            break
    if (thread_num == -1):
        print(
            "There are no poseidonos stack, could you check this core file is for poseidonos?")
        exit(1)

    print("We move thread %d frame %d" % (thread_num, callstack_num))
    gdb.execute("thread %d" % (thread_num), to_string=True)
    gdb.execute("f %d" % (callstack_num), to_string=True)


def switch_to_c_stack():
    print("Add C stack")
    output = gdb.execute("thread apply all bt", to_string=True)
    thread_num = -1
    callstack_num = 0
    line_list = output.split('\n')
    for line in line_list:
        if ("Thread" in line[0:8]):
            split_str = line.split()
            if (len(split_str) >= 2):
                thread_num = int(split_str[1])
        if ("in pos::" not in line and "#" in line):
            split_str = line.split()
            callstack_num = int(split_str[0].lstrip("#"))
            break
    if (thread_num == -1):
        print(
            "There are no C stack")
        exit(1)

    print("We move thread %d frame %d" % (thread_num, callstack_num))
    gdb.execute("thread %d" % (thread_num), to_string=True)
    gdb.execute("f %d" % (callstack_num), to_string=True)


def advance_ptr(parent_str, child_str):
    return_str = "(" + parent_str + "->" + child_str + ")"
    return return_str


def make_gdb_cmd_p(class_name, memory_addr):
    cmd = "p *" + "(" + class_name + "*" + ")" + str(memory_addr)
    return cmd
