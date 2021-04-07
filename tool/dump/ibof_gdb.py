#!/usr/bin/env python3

import os
import subprocess
import sys
import datetime
import argparse
import shutil
import gdb
current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1,current_path)
import core_dump_lib

class IbofosGdbCmd(gdb.Command):  

    def __init__(self):
        super(IbofosGdbCmd, self).__init__(
            "ibofgdb", gdb.COMMAND_USER
        ) 

    def show_debug_info(self):
        gdb.execute("p *ibofos::debugInfo")

    def complete(self, text, word):
        # We expect the argument passed to be a symbol so fallback to the
        # internal tab-completion handler for symbols
        return gdb.COMPLETE_SYMBOL

    def show_dev_context(self, ublock_addr):
        gdb.execute("set print repeats 0", to_string=True) 

        output = gdb.execute("p ((ibofos::UBlockDevice *) "+ ublock_addr +")->property", to_string=True) 
        print (output)

        output = gdb.execute("p ((ibofos::UBlockDevice *) "+ ublock_addr +")->contextArray", to_string=True) 
        
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

            output = gdb.parse_and_eval('((ibofos::UnvmeDeviceContext *)'+ memhex +')->pendingIoCount')
                    
            print ('\t=== Thread %d\'s Pending Io ===' % (index))
            print ('================================================')
            print ("pending IO count : ", output)
            
            output = gdb.parse_and_eval('((ibofos::UnvmeDeviceContext *)'+ memhex +')->pendingErrorList')
            
            print ("pending Error List : ", output)
            print ('================================================')
            index = index + 1

    def show_pending_io(self):
        
        output = gdb.execute('p ibofos::debugInfo->deviceManager->devices', to_string=True)
        if('length 0' in output):
            print ("There is no device on the system")
            return
        memaddr = output.split('=')[2].split(',')
        
        index = 0
        for mem in memaddr:
            memhex = mem.lstrip(' {').rstrip('\n }')
            print ("###### Device %d Information ###############" % (index))        
            self.show_dev_context(memhex)
            index = index + 1
            print ("############################################\n")
        
    def show_callback_list(self, callback_mem):

        callback_set = (gdb.execute ("p *((Callback *) %s) " % (callback_mem), to_string=True)).split('\n')        
        check_callee = False
        output_callee = ""
        output_subclass = ""
        for line_callback_set in callback_set:
            if("vptr.Event" in line_callback_set):
                output_subclass = line_callback_set.split('for')[1].split('+')[0].rstrip(' \n').lstrip(' ')
            if("callee" in line_callback_set):
                check_callee = True                              

            if(check_callee == True and "get()" in line_callback_set):
                output_callee = line_callback_set.split('=')[1].rstrip(' \n').lstrip(' ')
                check_callee = False

        print ("callback ptr : %s | callback subclass : %s | callback's callee : %s" %(callback_mem, output_subclass, output_callee) )                
        if (output_callee != "" and output_callee != "0x0" and output_callee != "0"):
            self.show_callback_list(output_callee)

    def show_stqh_first(self, mem, cnt):

        stqh_first_set = (gdb.execute ("p *(struct nvme_request *)%s" % (mem), to_string=True)).split('\n')
        #print (stqh_first_set)
        check_callee = False
        
        for line in stqh_first_set:
            if("cid" in line):
                cid = line.split('=')[1].rstrip(' ,\n').lstrip(' ')
                print (cnt, cid)
                break

        for line in stqh_first_set:
            if("stqe_next" in line):
            #if("tqe_next" in line):
                stqe_next = line.split('=')[1].rstrip(' ,\n').lstrip(' ')
                if(stqe_next != "0x0"):
                    self.show_stqh_first(stqe_next,cnt+1)
                break

    def show_pending_ubio(self):
        output = gdb.execute("p ibofos::gDumpSharedModulePtr", to_string=True)        
        pending_object_list = core_dump_lib.parse_shared_dump_header(current_path + '/../../')
        index = 0
        for pending_object in pending_object_list:
            if(pending_object == 'UBIO'):
                break
            index = index + 1
            
        memaddr = output.split('=')[1].split(',')[index].rstrip('\n }').lstrip(' {')
        if (memaddr == "0x0" or memaddr == "0"):
            print ("Ubio is not used before")
            return
        output = gdb.execute ("p ((ibofos::DumpSharedModule<ibofos::Ubio *, %d> *) %s)->dumpMap " \
                 % (index , memaddr), to_string=True)
        lines = output.split('\n')

        index = 0            
        for line in lines:
            if ("buffer" in line):
                ubio_memaddr = (line.split('=')[1].lstrip(' ').rstrip(' ,\n'))
                
                output_lba = gdb.parse_and_eval ("((Ubio *) %s)->pba.lba " % (ubio_memaddr))
                output_rba = gdb.parse_and_eval ("((Ubio *) %s)->sectorRba " % (ubio_memaddr))
                output_sync = gdb.execute ("p ((Ubio *) %s)->sync " % (ubio_memaddr), to_string = True).split('\n')
                output_sync_ele = ""

                for line_sync in output_sync:
                    if ("_M_i" in line_sync):
                        output_sync_ele = line_sync.split('=')[1].rstrip(' \n').lstrip(' ')

                output_callback = (gdb.execute ("p ((Ubio *) %s)->callback " % (ubio_memaddr), to_string=True)).split('\n')                
                
                check_callee = False
                output_callback_ele = ""
                for line_callback in output_callback:
                    if ("get()" in line_callback):
                        output_callback_ele = line_callback.split('=')[1].rstrip(' \n').lstrip(' ')
                        if (output_callback_ele == "0x0" or output_callback_ele == "0x0"):
                            break

                print ("ubio address : " + ubio_memaddr + " deviceLba : %s rba : %s sync : %s callback pointer : %s" \
                    % (output_lba, output_rba, output_sync_ele, output_callback_ele))    

                if (output_callback_ele != "" and output_callback_ele != "0x0" and output_callback_ele != "0x0"):     
                     self.show_callback_list (output_callback_ele)                                 
               
                index = index + 1
                print('')

        if (index == 0):        
            print ("There are no pending ubio")


    def show_pending_iocontext(self):
        output = gdb.execute("p ibofos::gDumpSharedModulePtr", to_string=True)        
        pending_object_list = core_dump_lib.parse_shared_dump_header(current_path + '/../../')
        index = 0
        for pending_object in pending_object_list:
            if(pending_object == 'IO_CONTEXT'):
                break
            index = index + 1
            
        memaddr = output.split('=')[1].split(',')[index].rstrip('\n }').lstrip(' {')
        output = gdb.execute ("p ((ibofos::DumpSharedModule<ibofos::IOContext *, %d> *) %s)->dumpMap " \
                              % (index , memaddr), to_string=True)
        lines = output.split('\n')
        
        index = 0
        for line in lines:
            if ("buffer" in line):
                iocontext_memaddr = (line.split('=')[1].lstrip(' ').rstrip(' ,\n'))
                print ("io context address : " + iocontext_memaddr) 
                index = index + 1

        if (index == 0):
            print ("There are no pending io context")

    def check_log_level_debug(self):
            output = str(gdb.parse_and_eval ("ibofos::debugInfo->logger->preferences->logLevel"))
            print (output)
            if ("spdlog::level::debug" in output):
                return True
            else:
                return False

    def get_in_memory_log(self):
        logger_list =core_dump_lib.parse_logger_header(current_path + '/../../')
        output_string_list = []
        index = 0
        for logger_module in logger_list:
            output = gdb.execute ("p (ibofos::debugInfo->logger->dumpModule[%d]->dumpQueue)\n" % (index), to_string=True)
            output_list = output.split('\n')
            output_string_list = output_string_list + output_list
            index = index + 1
        print (len(output_string_list))
        core_dump_lib.internal_parse_dump(output_string_list, "ibofos.inmemory.log")
        print ('Result file is \"ibofos.inmemory.log\"')

    def invoke(self, args, from_tty):
        # We can pass args here and use Python CLI utilities like argparse
        # to do argument parsing
        gdb.execute("set print pretty", to_string=True)
        gdb.execute("set print elements 0", to_string=True)
        gdb.execute("set height unlimited", to_string=True)
        print("Args Passed: %s" % args)

        print ("Add Ibofos stack")
        output = gdb.execute ("thread apply all bt", to_string=True)
        thread_num = -1
        callstack_num = 0
        line_list = output.split('\n')
        for line in line_list:
            if ("Thread" in line[0:8]):
                split_str = line.split()
                if (len(split_str) >= 2):
                    thread_num = int(split_str[1])
            if ("in ibofos::" in line):
                split_str = line.split()
                callstack_num = int(split_str[0].lstrip("#"))
                break
        if (thread_num == -1):
            print ("There are no ibofos stack, could you check this core file is for ibofos?")
            exit(1)

        print ("We move thread %d frame %d" % (thread_num, callstack_num))
        gdb.execute("thread %d" % (thread_num), to_string=True)
        gdb.execute("f %d" % (callstack_num), to_string=True)

        if(args == 'debuginfo'):
            self.show_debug_info()

        elif(args == 'pending io'):
            self.show_pending_io()

        elif(args == 'pending ubio'):
            if (self.check_log_level_debug() == False):
                print ("Log level is not set as debug")
            else:
                self.show_pending_ubio()

        elif (args == 'pending iocontext'):
            if (self.check_log_level_debug() == False):
                print ("Log level is not set as debug")
            else:
                self.show_pending_iocontext()

        elif ('callback' in args):
            split_args = args.split()[1]
            self.show_callback_list(split_args)
        elif ('log memory' == args):
            self.get_in_memory_log()
        else:
            print ("Help : ")
            help_f = open (current_path+'/README_IBOF_GDB')
            for line in help_f:
                print (line.rstrip('\n'))

def main():
    IbofosGdbCmd() 

if __name__=="__main__":
    main()
