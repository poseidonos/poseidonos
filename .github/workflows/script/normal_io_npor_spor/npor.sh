target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정


cd ${pos_working_dir}/test/regression/; sudo ./npor_ci_test.sh -f ${target_fabric_ip} -i 1

if [ $? -eq 0 ];
then
    echo "NPOR Test Success"
    cd ${pos_working_dir}/test/regression/; sudo echo 0 > nportest
    exit 0
else
    echo "\033[1;41mNPOR Test Failed\033[0m" 1>&2
    cd ${pos_working_dir}/test/regression/; sudo echo 1 > nportest
    exit 1
fi