#!/bin/bash
test_dir=$(readlink -f $(dirname $0))
ibof_root="/home/ibof/ibofos"
target_ip=127.0.0.1
target_fabric_ip=127.0.0.1
target_type="VM"
config_option=0
trtype="tcp"
port=1158
test_rev=0
result=0
failed=0
declare -a test_set_0=("volume" "npor" "multi_volume_io" "fault_tolerance" "spor")

texecc()
{
    echo "[target]" $@;
    sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${ibof_root}; sudo $@"
}

clear_result()
{
	texecc rm $ibof_root/test/regression/*test
}

build_setup()
{
    real_ip=$(ip -o addr show up primary scope global |
    	while read -r num dev fam addr rest; do echo ${addr%/*}; done)
    cmp_ip=\$\{real_ip:0:3\}
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

execute_test_set_0()
{
	
	for i in "${test_set_0[@]}"
	do
		echo "$i"
		sshpass -p bamboo ssh -q -tt root@${target_ip} "cd /home/ibof/ibofos/test/regression/; sudo ./${i}_ci_test.sh -f ${target_fabric_ip}"
		if [ $? -eq 0 ];
		then
			echo "$i test Success"
			sshpass -p bamboo ssh -q -tt root@${target_ip} "cd /home/ibof/ibofos/test/regression/; sudo echo 0 > ${i}test"
		else
			echo "\033[1;41m$i test Failed\033[0m" 1>&2
			sshpass -p bamboo ssh -q -tt root@${target_ip} "cd /home/ibof/ibofos/test/regression/; sudo echo 1 > ${i}test"
		fi
		sleep 10

		real_ip=$(ip -o addr show up primary scope global |
			while read -r num dev fam addr rest; do echo ${addr%/*}; done)
		cmp_ip=\$\{real_ip:0:3\}
		echo "real IP:${real_ip}, compare:${cmp_ip}"
		if [ ${cmp_ip} == "165" ]
		then
			echo "./clean_backup_165.sh -i $target_ip -n $i -r $test_rev"
    		$test_dir/clean_backup_165.sh -i $target_ip -n $i -r $test_rev
		else
			echo "./clean_backup.sh -i $target_ip -n $i -r $test_rev"
			$test_dir/clean_backup.sh -i $target_ip -n $i -r $test_rev
		fi
	done
}

print_result()
{
	echo "*****************************************************************"
	echo "*****************************************************************"
	for i in "${test_set_0[@]}"
	do
		echo "sshpass -p bamboo ssh -q -tt root@${target_ip} cd /home/ibof/ibofos/test/regression/; sudo cat ${i}test"
		result=`sshpass -p bamboo ssh -q -tt root@${target_ip} "cd /home/ibof/ibofos/test/regression/; sudo cat ${i}test"`
		if [ ${result} -eq 0 ];
		then
			echo "*****$i test Success"
		else
			echo "\033[1;41m*****$i test Failed\033[0m" 1>&2
			failed=1
		fi
	done

	if [ $failed -eq 0 ];
	then
		echo "***** All Test Success"
	else
		echo "\033[1;41m***** Some Test Failed\033[0m" 1>&2
	fi
	echo "*****************************************************************"
	echo "*****************************************************************"
}

print_help()
{
    echo "Script Must Be Called with Revision Number"
    echo "./postcommit_test.sh -i [target_ip=127.0.0.1] -f [target_fabric_ip=127.0.0.1] -t [target_type=VM] -r [test_revision] -c [config_option]"
}


while getopts "i:h:f:t:c:r:" opt
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
        ?) exit 2
            ;;
    esac
done

if [ $test_rev == 0 ] || [ $test_name == 0 ]
then
	print_help
	exit 1
fi

clear_result
build_setup
execute_test_set_0
print_result