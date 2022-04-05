target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

cd ${pos_working_dir}/test/system/io_path/; sudo ./precommit_loopback_test.sh -i ${target_fabric_ip}

if [ $? -eq 0 ];
then
    echo "Normal IO Test Success"
    cd ${pos_working_dir}/test/regression/; sudo echo 0 > iotest
    exit 0
else
    echo "\033[1;41mNormal IO Test Failed\033[0m" 1>&2
    cd ${pos_working_dir}/test/regression/; sudo echo 1 > iotest
    exit 1
fi
