target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정
target_type=$2

cd ${pos_working_dir}/test/system/qos/; sudo ./run_qos_test.sh -a ${target_fabric_ip} -l DSK -m 1 -v ${target_type}

if [ $? -eq 0 ];
then
    echo "Test Passed"
    exit 0
else
    echo "\033[1;41mTest Failed\033[0m" 1>&2
    exit 1
fi
