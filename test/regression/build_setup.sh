#!/bin/bash
ibof_root="/home/ibof/ibofos"
ibof_conf="/etc/pos"
target_ip=127.0.0.1
target_type="VM"
trtype="tcp"
port=1158
test_rev=0

texecc()
{
    echo "[target]" $@;
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${ibof_root}; sudo $@"
}

printVariable()
{
    if [ $test_rev == 0 ]
    then
        print_help
        exit 1
    fi

    echo "*****************************************************************"
    echo "*****     Script Info - Build Setup for CI                  *****"
    echo "*****     Must Main Variables Be Given                      *****"
    echo "*****************************************************************"
    echo "Target IP : $target_ip"
    echo "Transport Type : $trtype"
    echo "Port Number : $port"
    echo "PoseidonOS Root : $ibof_root"
    echo "Target Type : $target_type"
    echo "Config Option : $config_option"
    echo "Test Revision : $test_rev"
    echo "*****************************************************************"
    echo "*****************************************************************"
}

processKill()
{
    echo "Killing previously-running poseidonos..."
    texecc $ibof_root/test/script/kill_poseidonos.sh
}

repositorySetup()
{
    echo "Setting git repository..."
    texecc git fetch -p
    texecc git clean -dff
    texecc rm -rf *
    texecc git reset --hard $test_rev
    echo "Setting git repository done"
}

buildTest()
{
    texecc $ibof_root/script/pkgdep.sh
    texecc rm -rf /dev/shm/*
    texecc rm $ibof_root/bin/poseidonos

    texecc ./configure $config_option
    texecc cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=y -S lib -B lib
    if [ $target_type == "VM" ]
    then
        echo "Build For VM"
        texecc make CACHE=Y -j 4 -C lib
    elif [ $target_type == "PM" ]
    then
        echo "Build For PM"
        texecc make CACHE=Y -j 16 -C lib
    else 
        echo "Build For PSD"
        texecc make CACHE=Y -j 16 -C lib
    fi

    sshpass -p bamboo ssh -q -tt root@${target_ip} [[ -f $ibof_bin/ibofos_${test_rev} ]]

    echo "There is no binary with rev ${test_rev}"
    texecc make -j 4 clean
    if [ $target_type == "VM" ]
    then
        echo "Build For VM"
        texecc make CACHE=Y -j 4
    elif [ $target_type == "PM" ]
    then
        echo "Build For PM"
        texecc make CACHE=Y -j 16
    else 
        echo "Build For PSD"
        texecc make CACHE=Y -j 16
    fi
    

    texecc rm $ibof_conf/pos.conf
    texecc make install
    texecc make udev_install

    sshpass -p bamboo ssh -q -tt root@${target_ip} [[ -f $ibof_root/bin/poseidonos ]]
    if [ $? -eq 0 ]
    then
        echo "Build Success"
    else
        echo "Build Failed"
        exit 1
    fi
}

setupTest()
{
    texecc 'echo 1 > /proc/sys/vm/drop_caches'
    texecc ./script/setup_env.sh

    if [ $target_type == "VM" ]
    then
        texecc cp $ibof_root/config/ibofos_for_vm_ci.conf $ibof_conf/pos.conf
    fi

    texecc rmmod nvme_tcp
    texecc rmmod nvme_rdma
    texecc rmmod nvme_fabrics

    texecc modprobe nvme_tcp
    texecc modprobe nvme_rdma
}

print_help()
{
    echo "Script Must Be Called with Revision Number"
    echo "./build_setup.sh -i [target_ip=127.0.0.1] -t [target_type=VM] -r [test_revision] -c [config_option]"
}

while getopts "i:h:t:c:r:" opt
do
    case "$opt" in
        h) print_help
            ;;
        i) target_ip="$OPTARG"
            ;;
        t) target_type="$OPTARG"
            ;;
        c) config_option="$OPTARG"
            ;;
        r) test_rev="$OPTARG"
            ;;
        ?) exit 2
            ;;
    esac
done

printVariable
processKill
repositorySetup
buildTest
setupTest

echo "Build And Setup Success"
