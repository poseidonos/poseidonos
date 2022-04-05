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

import dump_buffer
import gdb_lib
import metafs

class DebugMetaCmd(gdb.Command):
    def __init__(self):
        super(DebugMetaCmd, self).__init__(
            "meta", gdb.COMMAND_USER
        )

    def show_debug_info(self):
        gdb.execute("p *pos::debugInfo")

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
        print("Args Passed: %s" % args)

        gdb_lib.switch_to_pos_stack()

        if(args == 'debuginfo'):
            self.show_debug_info()

        elif(args == 'metafs'):
            metafs.show_status()

        else:
            print("Help : ")
            help_f = open(current_path + '/README_DEBUG_META')
            for line in help_f:
                print(line.rstrip('\n'))

def main():
    DebugMetaCmd()

if __name__ == "__main__":
    main()
