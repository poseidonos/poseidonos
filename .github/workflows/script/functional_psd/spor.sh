target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

jq -r '.journal.enable |= true' /etc/pos/pos.conf > tmp.conf && mv tmp.conf /etc/pos/pos.conf

cd ${pos_working_dir}/test/regression/; sudo ./spor_ci_test.sh -f ${target_fabric_ip} --postcommit-test

if [ $? -eq 0 ];
then
    echo "Test Passed"
    exit 0
else
    echo "\033[1;41mTest Failed\033[0m" 1>&2
    exit 1
fi
