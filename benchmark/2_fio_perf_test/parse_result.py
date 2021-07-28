#!/usr/bin/env python3

# I use python 2.7 grammer 
import subprocess
from subprocess import check_call, call, check_output, Popen, PIPE
import random
import os
import sys
import re
import signal
import getopt
from datetime import datetime
from itertools import *
import csv
import itertools
from shutil import copyfile
import json
import sys


from collections import defaultdict


total = len(sys.argv)

# ./parse_result.py ResultFileName -> only single parse
# ./parse_result.py ResultFileName -> multiple parse


# Which column has the information from fio print
COL_RW        = 1
COL_BLOCK_SIZE= 3
COL_IODEPTH   = 9
COL_IOPS      = 1
COL_BW        = 3
IOPS_UNIT_LEN = 1
BW_UNIT_LEN   = 5
TEST_NR = 0
THOUSAND = 1000

# Please add config what you want to see 
# If there are no result in result File, that config will not be printed
# We use separate vector array to sort as shonw below.         
configReadWrite = ['write', 'read', 'randwrite', 'randread', 'randrw']
configBlockSize = ['512B', '4096B', '16KiB', '128KiB']
configIoDepth   = ['1', '4', '10', '32', '44', '64', '128', '256']

#If there is no config, skip print. 
dictReadWrite = dict()
dictBlockSize = dict()
dictIoDepth   = dict()
fileName = ""

reportDataIops = defaultdict(lambda : defaultdict(dict))
reportDataBW   = defaultdict(lambda : defaultdict(dict))

def parse(fileArg):            
    for testIndex in range(0, TEST_NR):
        if(total == 3):
            fileName     = fileArg + '_' + str(testIndex)
        else:
            fileName     = fileArg
            
        if not os.path.isfile(fileName):
            print("There is no such file : %s" % (fileName))
            exit(1)
        filePointer      = open(fileName)
        nextLineIsResult = 0

        for line in filePointer:
            #Get the parameters used in Fio.
            if("test0: (g=0" in line or "test0: (group" in line or "write:" in line or "read:" in line):
                if("test0: (g=0)" in line):
                    line2 = line.split("test0:")[1]
                    st = line2.split()
                    readWrite = st[COL_RW].split('=')[1].split(',')[0]
                    blockSize = st[COL_BLOCK_SIZE].split('-')[0]
                    ioDepth   = st[COL_IODEPTH].split('=')[1]
                    nextLineIsResult = 1
                #Get the result 
                elif("IOPS=" in line and nextLineIsResult == 1 and "fio" not in line):
                    st        = line.split()
                    iops      = st[COL_IOPS].split('=')[1]
                    bandwidth = st[COL_BW  ].split('(')[1]
                    iops      = iops[:-IOPS_UNIT_LEN]

                    if("k" in iops or "K" in iops):
                        iops = float(iops[:-IOPS_UNIT_LEN])
                        iops = iops*THOUSAND
                    else:
                        iops = float(iops)
                    if ("G" in bandwidth):
                        bandwidth = float(bandwidth[:-BW_UNIT_LEN])
                        bandwidth = bandwidth * THOUSAND
                    elif ("k" in bandwidth or "K" in bandwidth):
                        bandwidth = float(bandwidth[:-BW_UNIT_LEN])
                        bandwidth = bandwidth / float(1000)
                    else:
                        bandwidth = float(bandwidth[:-BW_UNIT_LEN])
                    
                    #result file containes the config? 
                    dictReadWrite[readWrite] = 1
                    dictBlockSize[blockSize] = 1
                    dictIoDepth[ioDepth]     = 1

                    # calculate the average 
                    if( testIndex == 0 ):
                        reportDataIops[ioDepth][readWrite][blockSize] = iops
                        reportDataBW[ioDepth][readWrite][blockSize]   = bandwidth
                    else:
                        reportDataIops[ioDepth][readWrite][blockSize] = \
                        ((reportDataIops[ioDepth][readWrite][blockSize] * testIndex) + iops) / (testIndex + 1)
                        reportDataBW[ioDepth][readWrite][blockSize]   = \
                        ((reportDataBW[ioDepth][readWrite][blockSize] * testIndex) + bandwidth) / (testIndex + 1)                                        
                    nextLineIsResult = 0

def print_result():
    for a in configIoDepth:
        for b in configReadWrite:            
            for c in configBlockSize:
                if(b in dictReadWrite and c in dictBlockSize and a in dictIoDepth):
                    print ("%.2f %.2f " % (reportDataIops[a][b][c]/1000,reportDataBW[a][b][c]), end = ' ')
        if(a in dictIoDepth):
            print ()

## Main Function.

def main():
    global TEST_NR
    if(total == 3): 
        TEST_NR = int(sys.argv[2],0)
    else:
        TEST_NR = 1
    if (total < 2):
        print ("####  Please Input Valid Parameter.. ####")
        print ("./parse_result [Prefix of FileName or FileName] [Number of Files]")
        exit(1)
#### Parse #####
    parse(sys.argv[1])
    print_result()
###  Print Result ###

if __name__=="__main__":
    main()
