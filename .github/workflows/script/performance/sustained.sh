target_ip=$1
target_fabric_ip_1=$2
target_fabric_ip_2=$3
initiator_ip_1=$4
initiator_ip_2=$5
root_dir=$6
test_rev=$7
initiator1_dir=$8
initiator2_dir=$9

target_pos_dir=${root_dir}
init1_spdk_dir=${initiator1_dir}/lib/spdk
init2_spdk_dir=${initiator2_dir}/lib/spdk
scenarios_output_dir=${root_dir}/test/system/benchmark/output

texecc()
{
    echo "[target]" $@;
    #sshpass -p bamboo ssh -q -tt root@${target_ip} "cd ${root_dir}; sudo $@"
    cd ${root_dir}; sudo $@
}

texecc pip3 install matplotlib

# use only 1/50 size
texecc rm -rf *
texecc git pull
texecc git checkout ${test_rev} -- .
cd ${root_dir}
sed -i '/SpdkNvmeNsGetSize/a diskSize = diskSize \/ 50\;' ${root_dir}/src/device/unvme/unvme_mgmt.cpp
texecc ./script/build_ibofos.sh -i

echo "change pm_fio_sustained.json"

texecc sed -i 's/\"target_id\"/\"root\"/g' ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i 's/\"target_pw\"/\"bamboo\"/g' ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i 's/\"init_id\"/\"root\"/g' ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i 's/\"init_pw\"/\"bamboo\"/g' ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i s/target_ssh/${target_ip}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i s/target_ip1/${target_fabric_ip_1}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i s/target_ip2/${target_fabric_ip_2}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i s/init1_ssh/${initiator_ip_1}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i s/init2_ssh/${initiator_ip_2}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json

texecc sed -i s/target_pos/${target_pos_dir}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i s/init1_spdk/${init1_spdk_dir}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i s/init2_spdk/${init2_spdk_dir}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json
texecc sed -i s/scenarios_output/${scenarios_output_dir}/g ${root_dir}/test/system/benchmark/config/pm_fio_sustained_github.json



echo "change fio_sustained.py"
texecc cd ${root_dir}
texecc sed -i 's/\"2000g\"/\"40g\"/g' ${root_dir}/test/system/benchmark/scenario/fio_sustained.py
texecc sed -i 's/\"70\"/\"0\"/g' ${root_dir}/test/system/benchmark/scenario/fio_sustained.py

texecc python3 ${root_dir}/test/system/benchmark/benchmark.py --config=config/pm_fio_sustained_github.json