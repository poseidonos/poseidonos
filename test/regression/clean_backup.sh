#!/bin/bash
pos_root_dir="/home/ibof/"
pos_working_dir="${pos_root_dir}/ibofos"
pos_core="/etc/pos/core"
pos_log="/etc/pos/log"
target_ip=0
trtype="tcp"
port=1158

texecc()
{
    echo "[target]" $@;
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${pos_working_dir}; sudo $@"
}

processCheck()
{
    rm -rf processList_${target_ip}
    sshpass -p bamboo ssh -q -tt root@${target_ip} ps -ef | grep poseidonos > processList_${target_ip}
    cat processList_${target_ip}
    rm -rf processList_${target_ip}
}

printVariable()
{
    if [ $target_ip == 0 ]
    then
        print_help
        exit 1
    fi

    echo "*****************************************************************"
    echo "*****     Script Info - Clean Bakcup for CI                 *****"
    echo "*****     Must Main Variables Be Given                      *****"
    echo "*****************************************************************"
    echo "Target IP : $target_ip"
    echo "Transport Type : $trtype"
    echo "Port Number : $port"
    echo "PoseidonOS Root : $pos_working_dir"
    echo "Test Name : $test_name"
    echo "Test Revision : $test_rev"
}

coreDump()
{
    echo "Deleting previously-generated core dump files.."
    texecc pkill -11 poseidonos
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd $pos_working_dir/tool/dump/; sudo ./trigger_core_dump.sh crashed"

    sshpass -p bamboo ssh -q -tt root@${target_ip} [[ ! -d $pos_core/$test_name/$test_rev ]]
    if [ $? -eq 0 ]
    then
        texecc mkdir -p $pos_core/$test_name/$test_rev
    fi

    echo "Copying core dump files to $pos_core/$test_name/$test_rev"
    texecc cp $pos_working_dir/tool/dump/*.tar.gz* $pos_core/$test_name/$test_rev
    texecc rm $pos_core/*
    texecc rm $pos_working_dir/tool/dump/*.tar.gz*
}

backupLog()
{
    sshpass -p bamboo ssh -q -tt root@${target_ip} [[ ! -d $pos_log/$test_name/$test_rev ]]
    if [ $? -eq 0 ]
    then
        texecc mkdir -p $pos_log/$test_name/$test_rev
    fi

    echo "Copying log files to $pos_log/$test_name/$test_rev"
    texecc cp /var/log/pos/* $pos_log/$test_name/$test_rev/
}

resetConfig()
{
    echo "Rescan PCI Devices..."
    echo 1 > /sys/bus/pci/rescan

}

print_help()
{
    echo "Script Must Be Called with Variables"
    echo "./clean_backup.sh -i [target_ip] -n [test_name] -r [revision] -d [working directory]"
}

while getopts "i:h:n:r:d:" opt
do
    case "$opt" in
        h) print_help
            ;;
        i) target_ip="$OPTARG"
            ;;
        n) test_name="$OPTARG"
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

processCheck
printVariable
backupLog
coreDump
resetConfig

echo "Clean and Backup Success"
