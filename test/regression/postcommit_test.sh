#!/bin/bash

#####################################################################################
# iBoF OS Post-commit test 
#    - Note that each initiator and target server environment is set up manually
#	(using ./script/setup_env.sh)
#    - Default configuration is for VM CI target server
#####################################################################################

rootdir=$(readlink -f $(dirname $0))/../..

print_help()
{
cat << EOF
Post-commit test

Synopsis
./postcommit_test.sh [target_ip] [transport] [target_fabric_ip] [target_port]
    
Description
	target_ip
		target server ip for ssh access
	transport
		RDMA or TCP
	target_fabric_ip/target_port
		target server ip/port where ibof would be running
			       
	If not specified, it is set to 10.1.11.254 tcp 10.100.11.254 1158
				
EOF
	exit 0
}

transport=tcp
target_ip=10.1.11.254
target_fabric_ip=10.100.11.254
target_port=1158
exec_mode=0
cwd="/home/ibof/ibofos/"
test_rev=$(git rev-parse HEAD)
spdk_rpc="/home/ibof/ibofos/lib/spdk-19.10/scripts/rpc.py"
nss="nqn.2019-04.ibof:subsystem1"

if [ ! -z "$1" ];
then
	transport=$1
	target_ip=$2
	target_fabric_ip=$3
	target_port=$4
fi

# Execution in initiator server (this machine)
iexecc()
{
	case ${exec_mode} in
	0) # default test
		echo "[initiator]" $@;
		$@
		;;
	1) # echo command
		echo "[initiator]" $@;
		;;
																	
	esac
}

# Execution in target server
texecc()
{
	case ${exec_mode} in
	0) # default test
		echo "[target]" $@;
		sshpass -p ibof ssh -tt root@${target_ip} "cd ${cwd}; sudo $@"
		;;
	1) # echo command
		echo "[target]" $@;
		;;
																	
	esac
}

setup_initiator()
{
	iexecc cd $rootdir/lib && ./build_ibof_lib.sh dpdk && ./build_ibof_lib.sh spdk && cd -
}

setup_target()
{
	texecc git reset --hard
	texecc git pull
	texecc git checkout ${test_rev}
	texecc ./script/build_ibofos.sh
}

start_ibofos()
{
	texecc ./script/start_ibofos.sh

	iexecc sleep 3;

	texecc ${spdk_rpc} nvmf_subsystem_create ${nss} -a -s IBOF00000000000001
	texecc ${spdk_rpc} bdev_malloc_create -b uram0 1024 4096
	texecc ./bin/cli request scan_dev
	texecc ./bin/cli request create_array -b uram0 -d unvme-ns-0,unvme-ns-1,unvme-ns-2 -s unvme-ns-3
	texecc ./bin/cli request mount_ibofos
	texecc ./bin/cli request create_vol --name vol1 --size 21474836480
	texecc ./bin/cli request mount_vol --name vol1
	texecc ${spdk_rpc} nvmf_create_transport -t TCP -u 131072 -p 4 -c 0
	texecc ${spdk_rpc} nvmf_subsystem_add_listener ${nss} -t ${transport} -a ${target_fabric_ip} -s ${target_port}
	texecc ${spdk_rpc} get_nvmf_subsystems
}

run_test()
{
	iexecc ./fio_bench.py -t ${transport} -i ${target_fabric_ip} -p ${target_port} -n 1
	# more test to be added here
}

if [[ $1 == "-h" ]] || [[ $1 == "--help" ]]; then
	print_help;
fi

echo "----------------------------------------------------------------"
echo "Start post-commit test"
echo "target ip: ${target_ip}"
echo "target fabric ip: ${target_fabric_ip}"
echo "target port: ${target_port}"
echo "target ibof direcotry: ${cwd}"
echo "test ibofos revision: ${test_rev}"
echo "----------------------------------------------------------------"

setup_initiator
setup_target

start_ibofos	
run_test
texecc ./test/script/kill_ibofos.sh
