#!/bin/bash
#
# nvmf_target.sh
#
    NVMF_TARGET_DIR=../../nvmf/target/c/
	NVMF_TARGET_APP=$NVMF_TARGET_DIR/ibof_nvmf_tgt
    SPDK_DIR=../../../../lib/spdk
    SUBSYSTEM_NUM=8
    cd $NVMF_TARGET_DIR
    make
    cd -
	echo "run $NVMF_TARGET_APP with rpc commands"
	#Note : below do the same configuration with nqn_ibof.conf
	$NVMF_TARGET_APP -m 0x3ff &
	sleep 2

   $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t TCP -u 131072 -n 4096 -b 64
    for i in `seq 1 $SUBSYSTEM_NUM`
    do    
       $SPDK_DIR/scripts/rpc.py bdev_null_create Null$i 4096 4096
#        $SPDK_DIR/scripts/rpc.py bdev_malloc_create -b Null$i 4096 4096
    done        

    for i in `seq 1 $SUBSYSTEM_NUM`
    do    
       $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.ibof:subsystem$i -a -s IBOF0000000000000$i -d IBOF_VOLUME
    done

    for i in `seq 1 $SUBSYSTEM_NUM`
    do    
       $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_ns nqn.2019-04.ibof:subsystem$i Null$i
    done       

    for i in `seq 1 $SUBSYSTEM_NUM`
    do    
       $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.ibof:subsystem$i -t tcp -a 172.16.1.1 -s 1158
    done

exit 0
