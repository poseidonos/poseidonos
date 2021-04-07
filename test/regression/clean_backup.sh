#!/bin/bash
ibof_root="/home/ibof/ibofos"
ibof_core="/etc/ibofos/core"
ibof_log="/etc/ibofos/log"
target_ip=0
target_fabric_ip=0
trtype="tcp"
port=1158

texecc()
{
    echo "[target]" $@;
    sshpass -p bamboo ssh -tt root@${target_ip} "cd ${ibof_root}; sudo $@"
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
    echo "Target Fabric IP : $target_fabric_ip"
    echo "Transport Type : $trtype"
    echo "Port Number : $port"
    echo "iBoFOS Root : $ibof_root"
    echo "Test Name : $test_name"
    echo "Test Revision : $test_rev"
}

processCheck()
{
    result=`sshpass -p bamboo ssh -tt root@$target_ip pgrep ibofos -c`
    if [ $result -ne 0 ];
    then
        echo "\033[1;41mibofos was not terminated\033[0m" 1>&2
    fi
}

coreDump()
{
    texecc pkill -11 ibofos
    sshpass -p bamboo ssh -tt root@${target_ip} "cd $ibof_root/tool/dump/; sudo ./trigger_core_dump.sh crashed"

    sshpass -p bamboo ssh -tt root@${target_ip} [[ ! -d $ibof_core/$testname/$test_rev ]]
    if [ $? -eq 0 ]
    then
        texecc mkdir $ibof_core/$testname/$test_rev
    fi

    texecc cp $ibof_root/tool/dump/*.tar.gz* $ibof_core/$testname/$test_rev
    texecc rm $ibof_core/ibofos.core
}

backupLog()
{
    sshpass -p bamboo ssh -tt root@${target_ip} [[ ! -d $ibof_log/$testname/$test_rev ]]
    if [ $? -eq 0 ]
    then
        texecc mkdir $ibof_log/$testname/$test_rev
    fi

    texecc cp /var/log/ibofos/* $ibof_log/$testname/$test_rev/
}

resetConfig()
{
    echo 1 > /sys/bus/pci/rescan

}

print_help()
{
    echo "Script Must Be Called with Variables"
    echo "./clean_backup.sh -i [target_ip] -f [target_fabric_ip] -n [test_name] -r [revision] "
}

while getopts "i:h:f:n:r:" opt
do
    case "$opt" in
        h) print_help
            ;;
        i) target_ip="$OPTARG"
            ;;
        f) target_fabric_ip="$OPTARG"
            ;;
        n) test_name="$OPTARG"
            ;;
        r) test_rev="$OPTARG"
            ;;
        ?) exit 2
            ;;
    esac
done

printVariable
processCheck
coreDump
backupLog
resetConfig

echo "Clean and Backup Success"