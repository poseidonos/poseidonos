#!/bin/bash
ibof_root="/home/psd/"
while getopts "i:t:f:" opt
do
    case "$opt" in
        i) target_ip="$OPTARG"
            ;;
        f) target_fabric_ip="$OPTARG"
            ;;
        t) target_type="$OPTARG"
            ;;
        ?) exit 2
            ;;
    esac
done

result_file="health_result_${target_ip}"

printInfo()
{
    echo "*****************************************************************" >> $result_file
    echo "*****     Script Info - Health Check for CI                 *****" >> $result_file
    echo "*****     Based on Bamboo Agent Variable                    *****" >> $result_file
    echo "*****************************************************************" >> $result_file
    echo "Target IP : ${target_ip}" >> $result_file
    echo "Target Fabric IP : ${target_fabric_ip}" >> $result_file
    echo "Target Type : ${target_type}" >> $result_file
    echo "*****************************************************************" >> $result_file
    echo "*****************************************************************" >> $result_file
    newline
}

texecc()
{
    echo "[target]" $@;
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${ibof_root}; sudo $@"
}

newline()
{
    echo " " >> $result_file
}

setup()
{
    rm -rf $result_file
    texecc date > $result_file
    newline
}

processKill()
{
    echo ""
    echo "Executing Process Kill ..."
    echo ""

    texecc ps -ef | grep poseidonos >> $result_file
    newline
    texecc pkill -9 poseidonos >> $result_file
    newline
    texecc pkill -9 trigger_core_dump >> $result_file
    newline
    texecc rm -rf /etc/pos/core/* >> $result_file
    newline
    texecc ps -ef | grep poseidonos >> $result_file
    newline
}

cleanServer()
{
    texecc "sudo rm -rf /dev/shm/*"
    texecc "sudo df" >> $result_file
    newline
    texecc "echo '3' > /proc/sys/vm/drop_caches"
    texecc "find /etc/pos/bin -mtime +1"
    texecc "find /etc/pos/bin -mtime +1 -delete"
    texecc "find /etc/pos/log -mtime +1"
    texecc "find /etc/pos/log -mtime +1 -delete"
    texecc "find /etc/pos/core -mtime +1"
    texecc "find /etc/pos/core -mtime +1 -delete"
    texecc "mkdir -p /etc/pos/bin"
    texecc "mkdir -p /etc/pos/log"
    texecc "mkdir -p /etc/pos/core"
    newline
    texecc "sudo df" >> $result_file
    newline
    cat $result_file
}

network_module_check()
{
    texecc /home/psd/ibofos/test/regression/network_module_check.sh
}

setup
printInfo
processKill
cleanServer
network_module_check