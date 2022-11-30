root_dir=$(readlink -f $(dirname $0))/../../..
spdk_dir=${root_dir}/lib/spdk

fio \
--numjobs=1 \
--thread=1 \
--ioengine=/remote_home/chkang0912/ibofos/lib/spdk/examples/nvme/fio_plugin/fio_plugin \
--direct=1 \
--rw=read \
--norandommap=0 \
--bs=4k \
--iodepth=1 \
--size=100% \
--io_size=4k \
--serialize_overlap=0 \
--ramp_time=0 \
--runtime=0 \
--time_based=0 \
--group_reporting=1 \
--name=job_Target01_nqn.2022-04.pos\:subsystem001_1 \
--filename="trtype=tcp adrfam=IPv4 traddr=10.1.3.28 trsvcid=1158 subnqn=nqn.2022-04.pos\:subsystem001 ns=1"