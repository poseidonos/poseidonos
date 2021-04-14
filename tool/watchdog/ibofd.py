#!/usr/bin/python3

import os
import sys
import subprocess
import time
import psutil
import logging
import optparse
import signal

monitor_interval=10
logfile="/var/log/syslog"
run_post_process=True
run_mode=1

cwd = os.path.dirname(os.path.abspath(__file__))
ibofos_dir= cwd + "/../../"
process_lock="/var/tmp/spdk.sock.lock"

def run_pre_system():
    logger.info("prepare ibof system")
    os.chdir(ibofos_pre_proc['path'])
    return os.system(ibofos_pre_system['setup_env'])

def run_pre_proc():
    logger.info("check uram recovery")
    os.chdir(ibofos_pre_proc['path'])
    os.system(ibofos_pre_proc['urambackup'])
    return os.system(ibofos_pre_proc['cleanup'])

def run_post_proc(clean_start):
    logger.info("ibofos bringup script - clean_start:" + str(clean_start))
    time.sleep(3)
    os.chdir(ibofos_post_proc['path'])
    if clean_start:
        logger.error("RUN post process - Clean")
        ret = os.system(ibofos_post_proc['clean'])
    else:
        logger.error("RUN post process - Dirty")
        ret = os.system(ibofos_post_proc['dirty'])
    logger.error("RUN post process - ret={}".format(ret))

def run_process(cmd, clean_start):
    run_pre_proc()
    ret = os.system(cmd)
    if ret != 0:
        logger.error("RUN {} - FAILED ".format(ibofos_proc['name']))
    else:
        logger.error("RUN {} - SUCCESS ".format(ibofos_proc['name']))
        if run_post_process:
            run_post_proc(clean_start)

def check_process_exist(name):
    for p in psutil.process_iter(attrs=['ppid', 'cmdline', 'pid']):
        if name in str(p.info['cmdline']) and p.info['ppid'] == 1:
            return int(p.info['pid'])
    return 0

def check_clean_start():
    #Note - dirty mode ibofos start vased segfault. temporally blocks
    #return !os.path.exists(process_lock)
    return True

def check_system_setup_need():
    #Todo : check hugemem reservation
    return True

def do_work():
    while True:
        if check_process_exist(ibofos_proc['name']) == 0:
            logger.error("DETECT {} is not running".format(ibofos_proc['name']))
            clean_start = check_clean_start()
            run_process(ibofos_proc['cmd'],clean_start)
        #else:
            #logger.info("{} is running".format(ibofos_proc['name']))
        time.sleep(monitor_interval)
        pass

def daemonize():
    try:
        pid = os.fork()
        if pid > 0:
            print("ppid={}".format(pid))
            sys.exit(0)
    except OSError as error:
        print("Fail to fork. error: {} {}".format(error.errno, error.strerror))
        sys.exit()

    os.chdir('/')
    os.setsid()
    os.umask(0)

    try:
        pid = os.fork()
        if pid > 0:
            sys.exit(0)
    except OSError as error:
        sys.stderr.write('_Fork #2 failed: {0}\n'.format(error))
        sys.exit(1)

    sys.stdout.flush()
    sys.stderr.flush()
    si = open(os.devnull, 'r')
    so = open(os.devnull, 'w')
    se = open(os.devnull, 'w')
    os.dup2(si.fileno(), sys.stdin.fileno())
    os.dup2(so.fileno(), sys.stdout.fileno())
    os.dup2(se.fileno(), sys.stderr.fileno())

    do_work()

def init():
    logger.setLevel(logging.INFO)
    formatter = logging.Formatter("%(asctime)s-[%(name)s]-[%(levelname)s]-%(message)s")
    handler = logging.FileHandler(logfile)
    handler.setFormatter(formatter)
    logger.addHandler(handler) 
    logger.info("Run ibofd (logfile={}, monitor_interval={})".format(logfile, monitor_interval))
    #logger.debug("debug")
    #logger.warn("warn")
    #logger.error("error")
    signal.signal(signal.SIGINT, sigHandler)

def finalize():
    ibofd_pid = check_process_exist("ibofd.py")
    if ibofd_pid:
        logger.info("Finalize ibofd({})".format(ibofd_pid))
        os.kill(ibofd_pid, signal.SIGINT)
    ibofos_pid = check_process_exist(ibofos_proc['name'])
    if ibofos_pid:
        logger.info("Finalize ibofos({})".format(ibofos_pid))
        os.kill(ibofos_pid, signal.SIGKILL)

def sigHandler(sig, frame):
    ibofos_pid = check_process_exist(ibofos_proc['name'])
    if ibofos_pid:
        logger.info("Finalize ibofos({})".format(ibofos_pid))
        os.kill(ibofos_pid, signal.SIGKILL)
    sys.exit(0)

parser = optparse.OptionParser()
parser.add_option("-l", "--log", dest="log", help="set log file", default=logfile)
parser.add_option("-i", "--interval", dest="interval", help="monitoring interval", default=monitor_interval)
parser.add_option("-d", "--daemon", dest="daemon", help="run ibofd as process=0, daemon=1, service=2", default=run_mode)
parser.add_option("-f", "--finish", dest="finish", help="finalize both ibofd and ibofos", default=0)
parser.add_option("-p", "--path", dest="path", help="select ibofos root directory", default=ibofos_dir)
(options, args) = parser.parse_args()
logfile=options.log
run_mode=int(options.daemon)
finish=int(options.finish)
monitor_stopinterval=int(options.interval)
logger = logging.getLogger("ibofd")
ibofos_dir = options.path

ibofos_pre_system={
        'path': ibofos_dir + '/script',
        'setup_env':'./setup_env.sh',
        }
ibofos_pre_proc={
        'path': ibofos_dir + '/script',
        'urambackup':'./backup_latest_hugepages_for_uram.sh',
        'cleanup':'rm -rf /dev/shm/*'
        }
ibofos_proc={
        'name':'primary',
        'cmd': ibofos_dir + '/bin/ibofos primary &'
        }
ibofos_post_proc={
        'path': ibofos_dir + '/script',
        'clean':'./setup_ibofos_nvmf_volume.sh 1',
        'dirty':'./setup_ibofos_nvmf_volume.sh 0'
        }

if __name__ == '__main__':
    init()
    if finish:
        finalize()
    else:
        do_system_setup = check_system_setup_need()
        if do_system_setup:
            run_pre_system()

        if run_mode == 0:
            logger.info("run ibofd as process")
            do_work()
        elif run_mode == 1:
            logger.info("run ibofd as daemon")
            if check_process_exist("ibofd.py"):
                logger.error("ibofd is already running")
                sys.exit(1)
            daemonize()
        elif run_mode == 2:
            logger.info("run ibofd as service")
            daemonize()
        else:
            logger.info("invalid run mode" + run_mode)
            sys.exit(2);

