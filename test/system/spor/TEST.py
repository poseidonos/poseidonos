#!/usr/bin/env python3

import os
import random
import string
import subprocess
from datetime import datetime

## ibof path
ibof_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"

## log path
quick_mode = False
log_dir = os.path.dirname(os.path.abspath(__file__)) + "/log/"
log_date = datetime.today().strftime("%Y-%m-%d_%H:%M:%S")
output_log_path = ""
test_log_path = "spor_test.log"
ibof_log_path = ""

## pos setup
trtype = "tcp"
traddr = subprocess.check_output("hostname -I", shell=True).split()[-1].decode()
port = 1158
volSize = 2147483648

## mockfs setup
log_buffer_filename = "JournalLogBuffer"
mockfile = ibof_root + "test/system/spor/" + log_buffer_filename

## degugging
dump_log_buffer = False

## nvme setup
NVME_CLI_CMD = 'nvme'
NQN = 'nqn.2019-04.ibof:subsystem'

## fio setup
ioengine = ibof_root + "/lib/spdk-19.10/examples/nvme/fio_plugin/fio_plugin"

## verify patterns
patterns = []
for i in range(1,5,1):
    patterns.append('\\\"'+''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(16))+'\\\"')