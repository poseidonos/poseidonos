#!/usr/bin/env python3

import os
import sys
import datetime
import argparse
import shutil


def delete_file(args):
    result = None
    date = None
    datetimelib = datetime.datetime
    if (args.date != None):
        format_dates = ["%Y%m%d", "%Y%m%d_%H%M%S"]
        for format_date in format_dates:           
            try:            
                result = datetimelib.strptime(args.date, format_date)
            except:
                pass
        if(result is None):
            print ("Not valid input date. \n example : 20200904, 20200904_164044")
            exit(1)
        else:
            date = result 
    else:
        result = datetimelib.now() 
        date = result

    result = None
    if (args.time != None):
        
        if(args.date != None and "_" in args.date):
            print ("Time is defined duplicated. please indicate time option only one.")
            exit(1)

        datestr = date.strftime('%Y%m%d')
        date_and_time = datestr + "_" + args.time
        format_date = "%Y%m%d_%H%M%S"        
        try:
            result = datetimelib.strptime(date_and_time, format_date)
        except:
            pass
        if(result is None):
            print ("Not valid input time. \n example : 164044")
            exit(1)
        else:
            date = result 
    if (args.all == True):
        date = datetimelib.now()
    elif (args.recent != None):
        date = datetimelib.now() - datetime.timedelta(days=int(args.recent))

    file_list = sorted(os.listdir("./"))
    files_to_delete = []
    print("Target Date : ")
    print(date)
    
    for path in file_list:
        filename=os.path.basename(path)
        date_in_filenames = filename.split(".")
        if(len(date_in_filenames) > 3 and date_in_filenames):
            try:
                result =  datetimelib.strptime(date_in_filenames[2],"%Y%m%d_%H%M%S")                
            except:
                pass    
            if(result != None):
                if(result < date):
                    files_to_delete.append(filename)    

    if(len(files_to_delete) == 0):
        print ("There are no files to delete with given condition.")
        exit(0)
        
    if(args.autoreply == False):
        print("Target File to delete : ")
        print ("###################################################")
        for file_name in files_to_delete:
            print (file_name)
        print ("###################################################")                                
        print("Files decripted above will be deleted, Do you agree? (Y/N)")
        flag_string = input()
        if(not(flag_string == 'y' or flag_string == "Y" or flag_string == "yes" or flag_string == "Yes" or flag_string == "YES")):
            print("Files will not be deleted by user.")
            exit(1)

    
    files_to_delete.append('poseidonos.library')
    files_to_delete.append('poseidonos.lloutput')
    files_to_delete.append('pos.log')
    files_to_delete.append('library.tar.gz')
    files_to_delete.append('poseidonos.inmemory.log')
    files_to_delete.append('poseidonos.memory.log')
    files_to_delete.append('call_stack.info')
    files_to_delete.append('pending_io.info')

    for file_name in files_to_delete:
        print (file_name)                 
        os.system ('rm -rf "'+file_name+'"')   
    print("Above Files are deleted")    

            


def main():
    parser = argparse.ArgumentParser(
            description='Please add fio option to nvmf_etc_command.py')
    parser.add_argument('-r', '--recent',\
            help='Specify days for reserve. -r 5 means, the dump of recent 5 days is reserved.')
    parser.add_argument('-d', '--date',\
            help='Specify "date" as "YYYYMMDD" or "YYYYMMDD_HHMMSS". This script will delete before that day. (input day is excluded to delete) \n example : 20200904, 20200904_164044')
    parser.add_argument('-t', '--time',\
            help='Specify "time" as "HHMMSS". This script will delete before that time. (input time is excluded to delete) \n example : 164044')
    parser.add_argument('-a', '--all', action='store_true',
            help='delete all core file ')
    parser.add_argument('-y', '--autoreply', action='store_true',
            help='Without user reply, delete core file ')            
   
    args = parser.parse_args()
    
    if(args.all == False and args.date == None and args.time == None and args.recent == None):
        print("Please input argument. You can check argument rule with -h option.")
        exit(1)

    if(args.all == True and (args.date != None or args.time != None or args.recent != None)):
        print("Please -a or --all option solely without other options.")
        exit(1)

    if(args.recent == True and (args.date != None or args.time != None or args.all != None)):
        print("Please -r or --recent option solely without other options.")
        exit(1)
       
    delete_file(args)


if __name__=="__main__":
    main()
