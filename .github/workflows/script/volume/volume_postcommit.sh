target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

cd ${pos_working_dir}/test/regression/; sudo ./volume_ci_test.sh -f ${target_fabric_ip} -v 1

if [ $? -eq 0 ];
then
    echo "Volume Test Success"
    cd ${pos_working_dir}/test/regression/; echo 0 > volumetest
    exit 0
else
    echo "\033[1;41mVolume Test Failed\033[0m" 1>&2
    cd ${pos_working_dir}/test/regression/; echo 1 > volumetest
    exit 1
fi
