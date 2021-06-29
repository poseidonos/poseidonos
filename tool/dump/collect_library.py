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


#subprcess.call ('ls -al', shell=True)
LIB_OUTPUT = 'poseidonos.library'
LL_OUTPUT = 'poseidonos.lloutput'
os.system ('rm -rf '+LIB_OUTPUT)
os.system ('rm -rf '+LL_OUTPUT)
os.system ('ldd poseidonos > '+LIB_OUTPUT)

f = open(LIB_OUTPUT)
toCompressedLibs = ""
for line in f:
    splitString = line.split()
    filename = splitString[-2] 
    if ("linux-vdso.so" in filename):continue
      
    os.system ('ls -l '+filename+' > '+LL_OUTPUT)
    f2 = open(LL_OUTPUT)
    mark = ""
    originFile = ""
    for line in f2:
        mark = line.split()[-1][0]
        originFile  = line.split()[-1]       
    if(mark == "/"):
        toCompressedLibs = toCompressedLibs + " " + filename +  " " +originFile
    elif("not found" in line):
        toCompressedLibs = "../" + splitString[0].lstrip("../")
    else:
        toCompressedLibs = toCompressedLibs + " " + filename + " " + (os.path.dirname(filename)+'/'+originFile)        

print ('tar czf ' + sys.argv[1] + ' ' +toCompressedLibs)
os.system ('tar czf ' + sys.argv[1] + ' ' +toCompressedLibs)


