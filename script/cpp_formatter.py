#!/usr/bin/env python3

# ex:
# script/formatter.py -d directory
# scropt/formatter.py -f file

import os
import sys
import glob
import subprocess

import argparse

cwd = os.getcwd()

def parse_arguments(args):
    parser = argparse.ArgumentParser(description='code formatter')
    parser.add_argument('-f', '--file', default='',\
            help='Format with file')
    parser.add_argument('-d', '--directory', default='', help='Format with directory ')

    args = parser.parse_args()

    return args

def get_files(path):
    file_sets = ('*.c', '*.cpp', '*.hpp', '*.h')

    files = [glob.glob(path + "/" + file_name) for file_name in file_sets]
    files.sort()
    return [element for array in files for element in array]

def format_file(filename):
    command = "clang-format --style=file -i "
    command += filename
    subprocess.call(command, shell=True)

def format_directory(path):
    for (root, _, files) in os.walk(cwd+"/"+path):
        print("* Formatting files in " + root)
        if len(files) > 0:
            for filename in get_files(root):
                format_file(filename)

if __name__ == "__main__":
    args = parse_arguments(sys.argv)
    if args.file != '':
        format_file(args.file)
    elif args.directory != '':
        format_directory(args.directory)
    else:
        os.system("Please check your option")
