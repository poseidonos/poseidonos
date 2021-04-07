#!/bin/bash
# Note : increase NR_VOLUME will make multiple volumes and namespace
NR_VOLUME=2

sudo ../../../lib/spdk-19.10/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.ibof:subsystem1 -a -s IBOF00000000000001 -d IBOF_VOLUME
sudo ../../../lib/spdk-19.10/scripts/rpc.py bdev_malloc_create -b uram0 4096 4096

sudo ../../../bin/cli request scan_dev
sudo ../../../bin/cli request mount_arr -ft 1 -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2,unvme-ns-3 -s unvme-ns-4
for i in `seq 1 $NR_VOLUME`
do
        sudo ../../../bin/cli request create_vol --name vol$i --size 21474836480 --maxiops 100 --maxbw 0
        sudo ../../../bin/cli request mount_vol --name vol$i
done

sudo ../../../lib/spdk-19.10/scripts/rpc.py construct_ibof_bdev -b VolumeStub0 128 $(expr $NR_VOLUME + 1) 1
sudo ../../../lib/spdk-19.10/scripts/rpc.py nvmf_subsystem_add_ns nqn.2019-04.ibof:subsystem1 VolumeStub0
sudo ../../../lib/spdk-19.10/scripts/rpc.py nvmf_create_transport -t RDMA -u 131072 -p 4 -c 0
sudo ../../../lib/spdk-19.10/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.ibof:subsystem1 -t RDMA -a 172.16.1.1 -s 1158
sudo ../../../lib/spdk-19.10/scripts/rpc.py nvmf_get_subsystems
