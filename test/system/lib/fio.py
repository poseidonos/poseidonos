import os
import subprocess
import ibofos_constant
import ibofos

# def run_fio(readwrite, offset):
#     print("\tExecute FIO")
#     fio_bench = "../io_path/fio_bench.py"
#     global fio_proc
#     fio_proc = subprocess.Popen([fio_bench,
#                 "--bs", "4k", \
#                 "--readwrite", readwrite, \
#                 "-t", ibofos.TR_TYPE, \
#                 "-i", ibofos.TR_ADDR, \
#                 "--offset", str(offset), \
#                 "--io_size", str(ibofos_constant.SIZE_1GB * 20), \
#                 "--verify", "1"]\
#                 )
#     print("\tFIO start")

# def wait_fio():
#     global fio_proc
#     fio_proc.wait()

# def kill_fio():
#     global fio_proc
#     fio_proc.kill()
#     fio_proc.wait()


def start_fio(vol_id, runtime_sec, testname = ""):
    ip_addr = ibofos.TR_ADDR
    ns_id = str(vol_id + 1)
    if testname != "":
        test_name = testname
    else:
        test_name = "fio_custom_test"

    file_name = "trtype=tcp adrfam=IPv4 traddr=" + ip_addr + \
        " trsvcid=1158 subnqn=nqn.2019-04.ibof\:subsystem1 ns= " + ns_id
    ioengine_path = ibofos.IBOFOS_ROOT + "lib/spdk-19.10/examples/nvme/fio_plugin/fio_plugin"
    fio_proc = subprocess.Popen(["fio",
        "--ioengine=" + ioengine_path,\
        "--runtime=" + str(runtime_sec), \
        "--bs=4096", \
        "--iodepth=128",\
        "--readwrite=write",\
        "--offset=0",\
        "--bs_unaligned=1",\
        "--bs=4096",\
        "--verify=md5",\
        "--serialize_overlap=1",\
        "--time_based",\
        "--numjobs=1",\
        "--thread=1",\
        "--group_reporting=1",\
        "--direct=1",\
        "--name=" + test_name, \
        "--filename=" + file_name]\
        )

    return fio_proc

def wait_fio(fio_proc):
    fio_proc.wait()

def stop_fio(fio_proc):
    if fio_proc.poll() is None:
        fio_proc.kill()
        fio_proc.wait()