target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

jq -r  '.journal.enable |= true'  /etc/pos/ibofos.conf > tmp.conf && mv tmp.conf /etc/pos/conf/ibofos.conf

cd ${pos_working_dir}/test/regression; sudo ./spor_ci_test.sh -f ${target_fabric_ip} --precommit-test

if [ $? -eq 0 ];
then
    echo "SPOR Test Success"
    sudo cp ${pos_working_dir}/config/ibofos_for_vm_ci.conf /etc/pos/pos.conf
    exit 0
else
    echo "\033[1;41mFinish SPOR test dump\033[0m" 1>&2
    sudo cp ${pos_working_dir}/config/ibofos_for_vm_ci.conf /etc/pos/pos.conf
    exit 1
fi