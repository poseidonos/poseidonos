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
    log_path = "/var/log/pos"
    log_file = "pos.*"

    def __init__(self):
        super(DebugMetaCmd, self).__init__(
            "meta", gdb.COMMAND_USER
        )


    def show_singleton_info(self):
        print("- singletonInfo.allocatorService")
        gdb.execute('p *(pos::AllocatorService*)singletonInfo.allocatorService')
        print("- singletonInfo.arrayManager")
        gdb.execute('p *(pos::ArrayManager*)singletonInfo.arrayManager')
        print("- singletonInfo.mapperService")
        gdb.execute('p *(pos::MapperService*)singletonInfo.mapperService')


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

        if(args == 'status'):
            arrayCount = gdb.execute("p singletonInfo.arrayManager.arrayList.size()", to_string=True)
            arrayCount = int(arrayCount.split('=')[1].strip(',\n {}'))
            if arrayCount != 0:
                arrayList = gdb.execute("p singletonInfo.arrayManager.arrayList", to_string=True)
                arrayList = arrayList.split('\n')
                for idx in range(1, arrayCount + 1):
                    arrayName = arrayList[idx].split('=')[0].strip()
                    arrayPtr = arrayList[idx].split('=')[1].strip()
                    print("- array status " + arrayName)
                    gc = gdb.execute("p ((pos::ArrayComponents*)" + arrayPtr + ").gc.isRunning", to_string=True)
                    gc = gc.split('=')[1].strip()
                    print("gc module is running: " + gc)
                    gc = gdb.execute("p ((pos::ArrayComponents*)" + arrayPtr + ").gc.gcStatus.gcRunning", to_string=True)
                    gc = gc.split('=')[1].strip()
                    print("gc is running: " + gc)
                    rebuilderPtr = gdb.execute("p ((pos::ArrayComponents*)" + arrayPtr + ").arrayRebuilder", to_string=True)
                    rebuilderPtr = rebuilderPtr.split(' ')[-1].strip()
                    rebuilder = gdb.execute("p ((pos::ArrayRebuilder*)" + rebuilderPtr + ").jobsInProgress", to_string=True)
                    rebuilder = rebuilder.split('=')[1].strip()
                    print("count of arrays in rebuilding: " + rebuilder)
            else:
                print("- no array is running")

        elif(args == 'metafs'):
            metafs.show_status()

        elif(args == 'lastlog'):
            args = args.replace('lastlog ', '').strip()
            print("log path: " + self.log_path)
            print("log file: pos.log")
            gdb.execute('shell cat ' + self.log_path + '/pos.log|tail -n 10')

        elif(args.find('lastlog ') == 0):
            args = args.replace('lastlog ', '').strip()
            print("log path: " + self.log_path)
            print("log file: pos.log")
            gdb.execute('shell cat ' + self.log_path + '/pos.log|tail -n "' + args + '"')

        elif(args.find('mem ') == 0):
            args = args.split(' ')
            gdb.execute('x/' + args[2].strip() + 'wx ' + args[1].strip())

        elif(args.find('search ') == 0):
            args = args.replace('search ', '').strip()
            print("log path: " + self.log_path)
            print("log file: " + self.log_file)
            gdb.execute('shell cat ' + self.log_path + '/' + self.log_file + '|grep "' + args + '"')

        elif(args.find('logpath ') == 0):
            print("current log path: " + self.log_path)
            self.log_path = args.replace('logpath ', '').strip()
            print("set log path: " + self.log_path)

        elif(args.find('logfile ') == 0):
            print("current log file: " + self.log_file)
            self.log_file = args.replace('logfile ', '').strip()
            print("set log file: " + self.log_file)

        else:
            print("Help : ")
            help_f = open(current_path + '/README_DEBUG_META')
            for line in help_f:
                print(line.rstrip('\n'))


def main():
    DebugMetaCmd()


if __name__ == "__main__":
    main()
