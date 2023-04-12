target_fabric_ip="$1"
pos_working_dir="$2" #추후 수정
initiator_1_ip="$3"
initiator_1_passwd="$4"
target_passwd="$5"
initiator_ibofos_root="/home/smrc/ibofos"

iexecc()
{
    sshpass -p $initiator_1_passwd ssh -tt root@${initiator_1_ip} "cd ${cwd}; sudo $@"
}

# init
iexecc rmmod nvme_tcp
iexecc rmmod nvme_rdma
iexecc rmmod nvme_fabrics
iexecc modprobe nvme_tcp
iexecc modprobe nvme_rdma
iexecc modprobe nvme_fabrics
iexecc "echo 0 > /proc/sys/kernel/randomize_va_space"

# target
cd ${pos_working_dir}/test/system/filesystem/;
sudo ./filebench_test.py -t ${target_fabric_ip} -f ${target_fabric_ip} -i ${initiator_1_ip} -r ${pos_working_dir} --target_pw ${target_passwd} --initiator_pw ${initiator_1_passwd} --initiator_ibofos_root ${initiator_ibofos_root}

if [ $? -eq 0 ];
then
    echo "Test Passed"
    exit 0
else
    echo "\033[1;41mTest Failed\033[0m" 1>&2
    exit 1
fi
