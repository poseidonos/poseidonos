target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

cd ${pos_working_dir}/test/regression; sudo ./spor_ci_test.sh -f ${target_fabric_ip} --postcommit-test

if [ $? -eq 0 ];
then
    echo "SPOR Test Success"
    cd ${pos_working_dir}/test/regression; sudo echo 0 > sportest
    exit 0
else
    echo "\033[1;41mSPOR Test Failed\033[0m" 1>&2
    cd ${pos_working_dir}/test/regression; sudo echo 1 > sportest
    exit 1
fi