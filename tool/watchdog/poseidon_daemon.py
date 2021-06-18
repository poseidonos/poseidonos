#!/usr/bin/python3

import os
import sys
import subprocess
import time
import psutil
import logging
import optparse
import signal

monitor_interval = 10
logfile = "/var/log/syslog"
run_post_process = False
run_mode = 1

cwd = os.path.dirname(os.path.abspath(__file__))
pos_dir = cwd + "/../../"
process_lock = "/var/tmp/spdk.sock.lock"


def run_pre_system():
    logger.info("prepare poseidon system")
    os.chdir(pos_pre_proc['path'])
    return os.system(pos_pre_system['setup_env'])


def run_pre_proc():
    logger.info("check uram recovery")
    os.chdir(pos_pre_proc['path'])
    os.system(pos_pre_proc['urambackup'])
    return os.system(pos_pre_proc['cleanup'])


def run_post_proc():
    time.sleep(3)
    os.chdir(pos_post_proc['path'])
    logger.error("RUN post process")
    ret = os.system(pos_post_proc['cmd'])
    logger.error("RUN post process - ret={}".format(ret))


def run_process(cmd):
    run_pre_proc()
    ret = os.system(cmd)
    if ret != 0:
        logger.error("RUN {} - FAILED ".format(pos_proc['name']))
    else:
        logger.error("RUN {} - SUCCESS ".format(pos_proc['name']))
        if run_post_process:
            run_post_proc()


def check_process_exist(name):
    for p in psutil.process_iter(attrs=['ppid', 'cmdline', 'pid']):
        if name in str(p.info['cmdline']) and p.info['ppid'] == 1:
            return int(p.info['pid'])
    return 0


def check_system_setup_need():
    ret = check_process_exist("poseidonos")
    if ret == 0:
        return True
    else:
        return False


def do_work():
    while True:
        if check_process_exist(pos_proc['name']) == 0:
            logger.error("DETECT {} is not running".format(pos_proc['name']))
            run_process(pos_proc['cmd'])
        # else:
            # logger.info("{} is running".format(pos_proc['name']))
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
    logger.info("Run poseidon_daemon (logfile={}, monitor_interval={})".format(logfile, monitor_interval))
    # logger.debug("debug")
    # logger.warn("warn")
    # logger.error("error")
    signal.signal(signal.SIGINT, sigHandler)


def finalize():
    posd_pid = check_process_exist("poseidon_daemon.py")
    if posd_pid:
        logger.info("Finalize poseidon_daemon({})".format(posd_pid))
        os.kill(posd_pid, signal.SIGINT)
    pos_pid = check_process_exist(pos_proc['name'])
    if pos_pid:
        logger.info("Finalize poseidonos({})".format(pos_pid))
        os.kill(pos_pid, signal.SIGKILL)


def sigHandler(sig, frame):
    pos_pid = check_process_exist(pos_proc['name'])
    if pos_pid:
        logger.info("Finalize poseidonos({})".format(pos_pid))
        os.kill(pos_pid, signal.SIGKILL)
    sys.exit(0)


parser = optparse.OptionParser()
parser.add_option("-l", "--log", dest="log", help="set log file", default=logfile)
parser.add_option("-i", "--interval", dest="interval", help="monitoring interval", default=monitor_interval)
parser.add_option("-d", "--daemon", dest="daemon", help="run poseidon_daemon as process=0, daemon=1, service=2", default=run_mode)
parser.add_option("-f", "--finish", dest="finish", help="finalize both poseidon_daemon and poseidonos", default=0)
parser.add_option("-p", "--path", dest="path", help="select poseidonos root directory", default=pos_dir)
(options, args) = parser.parse_args()
logfile = options.log
run_mode = int(options.daemon)
finish = int(options.finish)
monitor_stopinterval = int(options.interval)
logger = logging.getLogger("poseidon_daemon")
pos_dir = options.path

pos_pre_system = {
        'path': pos_dir + '/script',
        'setup_env': './setup_env.sh',
        }
pos_pre_proc = {
        'path': pos_dir + '/script',
        'urambackup': './backup_latest_hugepages_for_uram.sh',
        'cleanup': 'rm -rf /dev/shm/*'
        }
pos_proc = {
        'name': 'poseidonos',
        'cmd': pos_dir + '/bin/poseidonos &'
        }
pos_post_proc = {
        'path': pos_dir + '/script',
        'cmd': './post_process'
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
            logger.info("run poseidon_daemon as process")
            do_work()
        elif run_mode == 1:
            logger.info("run poseidon_daemon as daemon")
            if check_process_exist("poseidon_daemon.py"):
                logger.error("poseidon_daemon is already running")
                sys.exit(1)
            daemonize()
        elif run_mode == 2:
            logger.info("run poseidon_daemon as service")
            daemonize()
        else:
            logger.info("invalid run mode" + run_mode)
            sys.exit(2)
