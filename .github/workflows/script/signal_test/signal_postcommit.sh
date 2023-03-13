target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정
cd ${pos_working_dir}/test/regression/; sudo ./signal_ci_test.sh -f ${target_fabric_ip}

if [ $? -eq 0 ];
then
    echo "Signal Test Success"
    cd ${pos_working_dir}/test/regression/; echo 0 > signaltest
else
    echo "\033[1;41mSignal Test Failed\033[0m" 1>&2
    cd ${pos_working_dir}/test/regression/; echo 1 > signaltest
    exit 1
fi

cd ${pos_working_dir}/test/regression/; sudo ./signal_during_rebuild.sh -f ${target_fabric_ip} -i 2

if [ $? -eq 0 ];
then
    echo "Signal During Rebuild Test Success"
    cd ${pos_working_dir}/test/regression/; echo 0 > signaltest
    exit 0
else
    echo "\033[1;41mSignal During Rebuild Test Failed\033[0m" 1>&2
    cd ${pos_working_dir}/test/regression/; echo 1 > signaltest
    exit 1
fi
