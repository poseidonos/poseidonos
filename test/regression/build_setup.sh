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

buildTest()
{
    texecc $pos_working_dir/script/pkgdep.sh
    texecc rm -rf /dev/shm/*
    texecc rm $pos_working_dir/bin/poseidonos

    texecc ./configure $config_option
    cwd=""
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${pos_working_dir}/lib; sudo cmake . -DSPDK_DEBUG_ENABLE=n -DUSE_LOCAL_REPO=y"
    texecc make -j 4 clean

    if [ $target_type == "PM" ]
    then
        echo "Build lib For PM"
        texecc make CACHE=Y -j 16 -C lib
        echo "Build For PM"
        texecc make CACHE=Y -j 16
    elif [ $target_type == "PSD" ]
    then
        echo "Build lib For PSD"
        texecc make CACHE=Y -j 16 -C lib
        echo "Build For PSD"
        texecc make CACHE=Y -j 16
    elif [ $target_type == "VM" ]
    then
        echo "copy air.cfg"
        texecc cp -f ${pos_working_dir}/config/air_ci.cfg ${pos_working_dir}/config/air.cfg

        echo "Build lib For VM"
        texecc make CACHE=Y -j 4 -C lib
        echo "Build For VM"
        echo "#### CHECK BINARY IN MASTER : ${master_bin_path}/${pos_bin_filename} ####"
        if [ ! -d ${master_bin_path} ]
        then
            echo "## mkdir ${master_bin_path}"
            sudo mkdir ${master_bin_path}
        fi
        if [ -f ${master_bin_path}/${pos_bin_filename} ]
        then
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
        else
            echo "There is no binary with rev ${test_rev}"
            echo "Build For VM"
            texecc make CACHE=Y -j 4

            sshpass -p bamboo ssh -q -tt root@${target_ip} [[ -f ${pos_working_dir}/bin/${pos_bin_filename} ]]
            if [ ! $? -eq 0 ]
            then
                echo "## ERROR: NO BUILT BINARY  ${target_ip}:/${pos_working_dir}/bin/${pos_bin_filename} "
                exit 1
            fi

            echo "## START TO COPY BIN FILES INTO MASTER : from ${target_ip}:${pos_working_dir}/bin to ${master_bin_path} ##"
            sshpass -p bamboo sudo scp root@${target_ip}:${pos_working_dir}/bin/* ${master_bin_path}/

            if [ -f ${master_bin_path}/${pos_bin_filename} ]
            then
                echo "#### COMPLETE TO COPY BINARY INTO MASTER ${master_bin_path}/${pos_bin_filename} "
            else
                echo "## ERROR: NO COPIED BINARY  ${master_bin_path}/${pos_bin_filename} "
                exit 1
            fi
        fi
    else
        echo "## ERROR: incorrect target type(VM/PM/PSD)"
        exit 1
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
        texecc cp $pos_working_dir/config/ibofos_for_vm_ci.conf $pos_conf/pos.conf
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
    echo "./build_setup.sh -i [target_ip=127.0.0.1] -t [target_type=VM] -r [test_revision] -c [config_option] -d [working directory]"
}

while getopts "i:h:t:c:r:d:" opt
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
