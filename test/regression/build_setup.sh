#!/bin/bash
pos_root_dir="/home/ibof/"
pos_working_dir="${pos_root_dir}ibofos"
pos_conf="/etc/pos"
target_ip=127.0.0.1
target_type="VM"
trtype="tcp"
port=1158
test_rev=0
master_bin_path="/psdData/pos-bin"
pos_bin_filename="poseidonos"
build_optimization="ON"
job_number=12
wt_enable=0

texecc()
{
    echo "[target]" $@;
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${pos_working_dir}; sudo $@"
}

printVariable()
{
    if [ $test_rev == 0 ]
    then
        print_help
        exit 1
    fi

    master_bin_path+=/${test_rev}
    echo "*****************************************************************"
    echo "*****     Script Info - Build Setup for CI                  *****"
    echo "*****     Must Main Variables Be Given                      *****"
    echo "*****************************************************************"
    echo "Target IP : $target_ip"
    echo "Transport Type : $trtype"
    echo "Port Number : $port"
    echo "PoseidonOS Root : $pos_working_dir"
    echo "Target Type : $target_type"
    echo "Config Option : $config_option"
    echo "Test Revision : $test_rev"
    echo "Master Bin Path: ${master_bin_path}"  
    echo "*****************************************************************"
    echo "*****************************************************************"
}

processKill()
{
    echo "Killing previously-running poseidonos..."
    texecc $pos_working_dir/test/script/kill_poseidonos.sh
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

setJobNumber()
{
    if [ $target_type == "PM" ] || [ $target_type == "PSD" ]
    then
        job_number=16
    elif [ $target_type == "VM" ]
    then
        job_number=12
    else
        echo "## ERROR: incorrect target type(VM/PM/PSD)"
        exit 1
    fi
}

buildLib()
{
    echo "Build lib For $target_type"
    texecc make CACHE=Y -j ${job_number} -C lib
}

buildPos()
{
    echo "Build For $target_type"
    texecc make CACHE=Y -j ${job_number}
}

getPos()
{
    echo "## START TO COPY BIN FILES INTO VM : from ${master_bin_path} to ${target_ip}:${pos_working_dir}/bin ##"
    sshpass -p bamboo ssh -q -tt root@${target_ip} [[ -d ${pos_working_dir}/bin ]]
    if [ ! $? -eq 0 ]
    then
        echo "## mkdir ${target_ip}:${pos_working_dir}/bin"
        texecc mkdir ${pos_working_dir}/bin
    else
        echo "## rm old ${target_ip}:${pos_working_dir}/bin files"
        texecc rm ${pos_working_dir}/bin/*
    fi
    sshpass -p bamboo sudo scp ${master_bin_path}/* root@${target_ip}:${pos_working_dir}/bin
}

backupPos()
{
    echo "## START TO COPY BIN FILES INTO MASTER : from ${target_ip}:${pos_working_dir}/bin to ${master_bin_path} ##"
    sshpass -p bamboo sudo scp root@${target_ip}:${pos_working_dir}/bin/* ${master_bin_path}/

    if [ -f ${master_bin_path}/${pos_bin_filename} ]
    then
        echo "#### COMPLETE TO COPY BINARY INTO MASTER ${master_bin_path}/${pos_bin_filename} "
    else
        echo "## ERROR: NO COPIED BINARY  ${master_bin_path}/${pos_bin_filename} "
        exit 1
    fi
}

buildTest()
{
    if [ $build_optimization == "OFF" ]
    then
        texecc sed -i 's/O2/O0/g' $pos_working_dir/Makefile
    fi

    texecc $pos_working_dir/script/pkgdep.sh
    texecc rm -rf /dev/shm/*
    texecc rm $pos_working_dir/bin/poseidonos

    texecc ./configure $config_option
    cwd=""
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${pos_working_dir}/lib; sudo cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=y -DASAN_ENABLE=n"
    texecc make -j ${job_number} clean

    if [ $target_type == "VM" ]
    then
        echo "copy air.cfg"
        texecc cp -f ${pos_working_dir}/config/air_ci.cfg ${pos_working_dir}/config/air.cfg
    fi

    if [ ! -d ${master_bin_path} ]
    then
        echo "## mkdir ${master_bin_path}"
        sudo mkdir ${master_bin_path}
    fi

    buildLib
    if [ $build_optimization == "OFF" ]
    then
        buildPos
    else
        echo "#### CHECK BINARY IN MASTER : ${master_bin_path}/${pos_bin_filename} ####"
        if [ -f ${master_bin_path}/${pos_bin_filename} ]
        then
            getPos
        else
            buildPos
        fi
    fi

    sshpass -p bamboo ssh -q -tt root@${target_ip} [[ -f ${pos_working_dir}/bin/${pos_bin_filename} ]]
    if [ ! $? -eq 0 ]
    then
        echo "## ERROR: NO BUILT BINARY  ${target_ip}:/${pos_working_dir}/bin/${pos_bin_filename} "
        exit 1
    fi

    if [ $target_type == "VM" ] && [ $build_optimization == "ON" ]
    then
        backupPos
    fi

    texecc rm $pos_conf/pos.conf
    texecc make install
    texecc make udev_install

    sshpass -p bamboo ssh -q -tt root@${target_ip} [[ -f $pos_working_dir/bin/poseidonos ]]
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
        if [ $wt_enable -eq 1 ]
        then
            texecc cp $pos_working_dir/config/ibofos_for_vm_ci_wt.conf $pos_conf/pos.conf
        else
            texecc cp $pos_working_dir/config/ibofos_for_vm_ci.conf $pos_conf/pos.conf
        fi
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
    real_ip=$(ip -o addr show up primary scope global |
        while read -r num dev fam addr rest; do echo ${addr%/*}; done)
    cmp_ip=${real_ip:0:3}
    echo "real IP:${real_ip}, compare:${cmp_ip}"
    if [ ${cmp_ip} == "165" ]
    then
        echo "./build_setup_165.sh -i [target_ip=127.0.0.1] -t [target_type=VM] -r [test_revision] -c [config_option] -d [working directory]"
    else
        echo "./build_setup.sh -i [target_ip=127.0.0.1] -t [target_type=VM] -r [test_revision] -c [config_option] -d [working directory]"
    fi    
}

while getopts "i:h:t:c:r:d:o:w" opt
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
        d) pos_root_dir="$OPTARG"
            pos_working_dir="${pos_root_dir}/ibofos"
            ;;
        o) build_optimization="$OPTARG"
            ;;
        w) wt_enable=1
            ;;
        ?) exit 2
            ;;
    esac
done

printVariable
processKill
repositorySetup
setJobNumber
buildTest
setupTest

echo "Build And Setup Success"
