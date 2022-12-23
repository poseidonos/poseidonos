#!/usr/bin/env python3

import os
import sys
current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/gdb_scripts/")
import core_dump_lib
import subprocess
import datetime
import argparse
import shutil
import gdb

import debug_info
import dump_buffer
import gdb_lib
import log_memory
import pending_object
import pending_callback
import pending_io
import pending_iocontext
import pending_ubio
import backend_io
import volume_io
import rba_info
import report


class PosGdbCmd(gdb.Command):

    def __init__(self):
        super(PosGdbCmd, self).__init__(
            "posgdb", gdb.COMMAND_USER
        )

    def show_singleton_info(self):
        gdb.execute("p *pos::singletonInfo")

    def complete(self, text, word):
        # We expect the argument passed to be a symbol so fallback to the
        # internal tab-completion handler for symbols
        return gdb.COMPLETE_SYMBOL

    def invoke(self, args, from_tty):
        # We can pass args here and use Python CLI utilities like argparse
        # to do argument parsing
        gdb.execute("set print pretty", to_string=True)
        gdb.execute("set print elements 0", to_string=True)
        gdb.execute("set height unlimited", to_string=True)
        gdb.execute("set max-value-size unlimited", to_string=True)
        print("Args Passed: %s" % args)

        gdb_lib.switch_to_pos_stack()

        if(args == 'singletonInfo'):
            self.show_singleton_info()

        elif(args == 'debug info'):
            debug_info.show_debug_info()

        elif(args == 'debug history'):
            debug_info.show_debug_info_history()

        elif(args == 'pending io'):
            pending_io.show_pending_io()

        elif(args == 'pending ubio'):
            if (gdb_lib.check_log_level_debug() is False):
                print("Log level is not set as debug")
            else:
                pending_ubio.show_pending_ubio()

        elif(args == 'pending callback'):
            if (gdb_lib.check_log_level_debug() is False):
                print("Log level is not set as debug")
            else:
                pending_callback.show_pending_callback()

        elif('dumpbuffer' in args):
            split_args = args.split()[1]
            dump_buffer.show_dump_buffer(split_args)

        elif (args == 'pending iocontext'):
            if (gdb_lib.check_log_level_debug() is False):
                print("Log level is not set as debug")
            else:
                pending_iocontext.show_pending_iocontext()

        elif ('callback' in args):
            split_args = args.split()[1]
            gdb_lib.show_callback_list(split_args)

        elif ('pending object' in args):
            split_args = args.split()[2]
            pending_object.show_pending_object(split_args)

        elif ('log memory' == args):
            log_memory.get_in_memory_log()

        elif ('backend io' == args):
            backend_io.backend_io()

        elif ('volume io' == args):
            volume_io.pending_volume_io()

        elif ('volume info' == args):
            volume_io.bdev_information()

        elif ('rba info' in args):
            array_id = args.split()[2]
            volume_id = args.split()[3]
            rba = args.split()[4]
            rba_info.rba_info(array_id, volume_id, rba)

        elif ('make report' in args):
            report.make_report()

        else:
            print("Help : ")
            help_f = open(current_path + '/README_POS_GDB')
            for line in help_f:
                print(line.rstrip('\n'))


def main():
    PosGdbCmd()


if __name__ == "__main__":
    main()
