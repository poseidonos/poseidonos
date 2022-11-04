target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정
target_type=$2

cd ${pos_working_dir}/test/system/qos/; sudo ./run_qos_test.sh -a ${target_fabric_ip} -l DSK -m 1 -v ${target_type}