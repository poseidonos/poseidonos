#!/bin/bash
ibof_root="/home/ibof/ibofos"
ibof_core="/etc/pos/core"
ibof_log="/etc/pos/log"
target_ip=0
trtype="tcp"
port=1158

texecc()
{
    echo "[target]" $@;
    sshpass -p bamboo ssh -tt root@${target_ip} "cd ${ibof_root}; sudo $@"
}

processCheck()
{
    rm -rf processList_${target_ip}
    sshpass -p bamboo ssh -tt root@${target_ip} ps -ef | grep ibofos > processList_${target_ip}
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
    echo "PoseidonOS Root : $ibof_root"
    echo "Test Name : $test_name"
    echo "Test Revision : $test_rev"
}

coreDump()
{
    echo "Deleting previously-generated core dump files.."
    texecc pkill -11 ibofos
    sshpass -p bamboo ssh -tt root@${target_ip} "cd $ibof_root/tool/dump/; sudo ./trigger_core_dump.sh crashed"

    sshpass -p bamboo ssh -tt root@${target_ip} [[ ! -d $ibof_core/$test_name/$test_rev ]]
    if [ $? -eq 0 ]
    then
        texecc mkdir -p $ibof_core/$test_name/$test_rev
    fi

    echo "Copying core dump files to $ibof_core/$test_name/$test_rev"
    texecc cp $ibof_root/tool/dump/*.tar.gz* $ibof_core/$test_name/$test_rev
    texecc rm $ibof_core/*
    texecc rm $ibof_root/tool/dump/*.tar.gz*
}

backupLog()
{
    sshpass -p bamboo ssh -tt root@${target_ip} [[ ! -d $ibof_log/$test_name/$test_rev ]]
    if [ $? -eq 0 ]
    then
        texecc mkdir -p $ibof_log/$test_name/$test_rev
    fi

    echo "Copying log files to $ibof_log/$test_name/$test_rev"
    texecc cp /var/log/pos/* $ibof_log/$test_name/$test_rev/
}

resetConfig()
{
    echo "Rescan PCI Devices..."
    echo 1 > /sys/bus/pci/rescan

}

print_help()
{
    echo "Script Must Be Called with Variables"
    echo "./clean_backup.sh -i [target_ip] -n [test_name] -r [revision] "
}

while getopts "i:h:n:r:" opt
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
        ?) exit 2
            ;;
    esac
done

processCheck
printVariable
coreDump
backupLog
resetConfig

echo "Clean and Backup Success"
