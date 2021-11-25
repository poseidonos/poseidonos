#!/bin/bash
test_dir=$(readlink -f $(dirname $0))
ibof_root="/home/ibof/ibofos"
ibof_bin="/etc/pos/bin"
ibof_conf="/etc/pos"
target_ip=127.0.0.1
target_type="VM"
config_option=0
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
    echo "*****     Script Info - Exec CI Test for CI                 *****"
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

build_setup()
{
    real_ip=$(ip -o addr show up primary scope global |
    while read -r num dev fam addr rest; do echo ${addr%/*}; done)
    cmp_ip=${real_ip:0:3}
    echo "real IP:${real_ip}, compare:${cmp_ip}"   
    if [ $config_option == 0 ]
    then
        if [ ${cmp_ip} == "165" ]
        then
            echo "./build_setup_165.sh -i $target_ip -t $target_type -r $test_rev"
            $test_dir/build_setup_165.sh -i $target_ip -t $target_type -r $test_rev
        else
            echo "./build_setup.sh -i $target_ip -t $target_type -r $test_rev"
            $test_dir/build_setup.sh -i $target_ip -t $target_type -r $test_rev
        fi
    else
        if [ ${cmp_ip} == "165" ]
        then
            echo "./build_setup_165.sh -i $target_ip -t $target_type -r $test_rev -c $config_option"
            $test_dir/build_setup_165.sh -i $target_ip -t $target_type -r $test_rev -c $config_option
        else
            echo "./build_setup.sh -i $target_ip -t $target_type -r $test_rev -c $config_option"
            $test_dir/build_setup.sh -i $target_ip -t $target_type -r $test_rev -c $config_option
        fi
    fi
}

exec_test()
{
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd /home/ibof/ibofos/test/regression/; sudo ./${test_name}_ci_test.sh -f ${target_fabric_ip}"
    if [ $? -eq 0 ];
    then
        echo "$test_name test Success"
        sshpass -p bamboo ssh -q -tt root@${target_ip} "cd /home/ibof/ibofos/test/regression/; sudo echo 0 > ${test_name}test"
    else
        echo "\033[1;41m$test_name test Failed\033[0m" 1>&2
        sshpass -p bamboo ssh -q -tt root@${target_ip} "cd /home/ibof/ibofos/test/regression/; sudo echo 1 > ${test_name}test"
    fi
}

clean_backup()
{
    real_ip=$(ip -o addr show up primary scope global |
        while read -r num dev fam addr rest; do echo ${addr%/*}; done)
    cmp_ip=${real_ip:0:3}
    echo "real IP:${real_ip}, compare:${cmp_ip}"
    if [ ${cmp_ip} == "165" ]
    then
        echo "./clean_backup_165.sh -i $target_ip -n $test_name -r $test_rev"
        $test_dir/clean_backup_165.sh -i $target_ip -n $test_name -r $test_rev
    else
        echo "./clean_backup.sh -i $target_ip -n $test_name -r $test_rev"
        $test_dir/clean_backup.sh -i $target_ip -n $test_name -r $test_rev
    fi
}

print_help()
{
    echo "Script Must Be Called with Revision Number"
    echo "./exec_ci_test.sh -i [target_ip=127.0.0.1] -f [target_fabric_ip] -t [target_type=VM] -r [test_revision] -n [test_name] -c [config_option]"
    echo "Test List : volume, npor, multi_volume_io, fault_tolerance, spor"
}

while getopts "i:h:f:t:c:r:n:" opt
do
    case "$opt" in
        h) print_help
            ;;
        i) target_ip="$OPTARG"
            ;;
        f) target_fabric_ip="$OPTARG"
            ;;
        t) target_type="$OPTARG"
            ;;
        c) config_option="$OPTARG"
            ;;
        r) test_rev="$OPTARG"
            ;;
        n) test_name="$OPTARG"
            ;;
        ?) exit 2
            ;;
    esac
done

build_setup
exec_test
sleep 10
clean_backup
