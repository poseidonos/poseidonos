#!/bin/bash
#
# nvmf_target.sh
#
    NVMF_TARGET_DIR=../../nvmf/target/c/
	NVMF_TARGET_APP=$NVMF_TARGET_DIR/ibof_nvmf_tgt
    SPDK_DIR=../../../../lib/spdk
    SUBSYSTEM_NUM=65
    cd $NVMF_TARGET_DIR
    make
    cd -
	echo "run $NVMF_TARGET_APP with rpc commands"
	#Note : below do the same configuration with nqn_ibof.conf
	$NVMF_TARGET_APP -m 0xff0001ffffffffffffff &
	sleep 2

    $SPDK_DIR/scripts/rpc.py nvmf_create_transport -t TCP -u 131072 -n 8192 -b 64
    for i in `seq 1 $SUBSYSTEM_NUM`
    do    
        $SPDK_DIR/scripts/rpc.py bdev_null_create Null$i 40960 512
#       sudo $SPDK_DIR/scripts/rpc.py bdev_malloc_create -b uram$i 512 512
    done        

    for i in `seq 1 $SUBSYSTEM_NUM`
    do    
       $SPDK_DIR/scripts/rpc.py nvmf_create_subsystem nqn.2019-04.pos:subsystem$i -a -s POS0000000000000$i -d POS_VOLUME
    done

    for i in `seq 1 $SUBSYSTEM_NUM`
    do           
       $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_ns nqn.2019-04.pos:subsystem$i Null$i
 #     $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_ns nqn.2019-04.pos:subsystem$i uram$i
    done       



    for i in `seq 1 33`
    do    
       $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t tcp -a 10.100.2.16 -s 1158
    done
    for i in `seq 34 65`
    do    
       $SPDK_DIR/scripts/rpc.py nvmf_subsystem_add_listener nqn.2019-04.pos:subsystem$i -t tcp -a 10.100.3.16 -s 1158
    done


exit 0
