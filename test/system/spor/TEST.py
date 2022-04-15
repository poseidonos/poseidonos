#!/usr/bin/env python3

import os
import subprocess
from datetime import datetime

# pos path
pos_root = os.path.dirname(os.path.abspath(__file__)) + "/../../../"
config_path = "/etc/pos/pos.conf"

# log path
log_dir = os.path.dirname(os.path.abspath(__file__)) + "/log/"
log_date = datetime.today().strftime("%Y-%m-%d_%H:%M:%S")
output_log_path = ""
test_log_path = "spor_test.log"
pos_log_path = ""

# Test mode
quick_mode = False
print_log_with_color = False
run_pos_manually = False
skip_fio_run = False
print_fio_result_to_log_file = False

# pos setup
trtype = "tcp"
traddr = subprocess.check_output("hostname -I", shell=True).split()[-1].decode()
port = 1158
volSize = 2147483648

# mockfs setup
log_buffer_filename = "JournalLogBuffer"
mockfile = pos_root + "test/system/spor/" + log_buffer_filename

# degugging
dump_log_buffer = False
dump_map = False

# nvme setup
NVME_CLI_CMD = 'nvme'
NQN = 'nqn.2019-04.pos:subsystem'

# fio setup
ioengine = pos_root + "/lib/spdk/examples/nvme/fio_plugin/fio_plugin"
EXPECTED_MEMORY_FIO = 0.4

# test setup
max_num_vol_per_array = 10
num_data_ssds_per_array = 3
num_spare_ssds_per_array = 0
run_time = 10
