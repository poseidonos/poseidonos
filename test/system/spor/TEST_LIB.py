#!/usr/bin/env python3

import argparse
import subprocess

import TEST_LOG
import TEST_SETUP_IBOFOS
import TEST

def parse_arguments(args):
    parser = argparse.ArgumentParser(description='Test journal feature with SPO')
    parser.add_argument('-f', '--fabric_ip', default=TEST.traddr,\
            help='Set target IP, default: ' + TEST.traddr)
    parser.add_argument('-t', '--transport', default=TEST.trtype,
            help='Set transport, default: ' + TEST.trtype)
    parser.add_argument('-p', '--port', type=int, default=TEST.port,
            help='Set port, default: ' + str(TEST.port))
    parser.add_argument('-l', '--log_dir', default=TEST.log_dir,\
            help='Set path for log file, default: ' + TEST.log_dir)
    parser.add_argument('-q', '--quick_mode', default=TEST.quick_mode , action='store_true',\
            help='Enable quick test mode, default: ' + str(TEST.quick_mode))
    
    args = parser.parse_args()
    TEST.traddr = args.fabric_ip
    TEST.trtype = args.transport
    TEST.port = args.port
    TEST.log_dir = args.log_dir
    TEST.quick_mode = args.quick_mode

def cleanup():
    TEST_SETUP_IBOFOS.cleanup_process()

def set_up(argv, test_name):
    parse_arguments(argv)
    TEST_LOG.setup_log(test_name)
    TEST_LOG.print_notice("[{} Started]".format(test_name))
    cleanup()

def tear_down(test_name):
    TEST_SETUP_IBOFOS.shutdown_ibofos()
    TEST_LOG.print_notice("[Test {} Completed]".format(test_name))