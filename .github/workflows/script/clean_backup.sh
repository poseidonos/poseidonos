#!/bin/bash
pos_working_dir=0
pos_core="/psdData/core"
target_ip=0
trtype="tcp"
port=1158

texecc()
{
    echo "[target]" $@;
    cd ${pos_working_dir}; sudo $@
}

processCheck()
{
    rm -rf processList_${target_ip}
    ps -ef | grep poseidonos > processList_${target_ip}
    cat processList_${target_ip}
    rm -rf processList_${target_ip}
}

printVariable()
{
    if [ $target_ip == 0 ] || [ $pos_working_dir == 0 ]
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
    echo "Plan Name : $plan_name"
    echo "Test Name : $test_name"
    echo "Test Revision : $test_rev"
}

coreDump()
{
    echo "Kill poseidonos to generate core dump files.."
    texecc pkill -11 poseidonos
    cd $pos_working_dir/tool/dump/; sudo ./trigger_core_dump.sh crashed
}

resetConfig()
{
    echo "Rescan PCI Devices..."
    echo 1 > /sys/bus/pci/rescan

}

print_help()
{
    echo "Script Must Be Called with Variables"
    real_ip=$(ip -o addr show up primary scope global |
      while read -r num dev fam addr rest; do echo ${addr%/*}; done)
    cmp_ip=${real_ip:0:3}
    echo "real IP:${real_ip}, compare:${cmp_ip}"
    if [ ${cmp_ip} == "165" ]
    then
        echo "./clean_backup_165.sh -i [target_ip] -p [plan_name] -n [test_name] -r [revision] -d [working directory]"
    else
        echo "./clean_backup.sh -i [target_ip] -p [plan_name] -n [test_name] -r [revision] -d [working directory]"
    fi
}

while getopts "i:h:p:n:r:d:" opt
do
    case "$opt" in
        h) print_help
            ;;
        i) target_ip="$OPTARG"
            ;;
        p) plan_name="$OPTARG"
            ;;
        n) test_name="$OPTARG"
            ;;
        r) test_rev="$OPTARG"
            ;;
        d) pos_working_dir="$OPTARG"
            ;;
        ?) exit 2
            ;;
    esac
done

processCheck
printVariable
coreDump
resetConfig

echo "Clean and Backup Success"
