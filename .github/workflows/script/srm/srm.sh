target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정

rm -rf /dev/shm/ibof_nvmf_trace*
cd ${pos_working_dir}/script/; ./setup_env.sh
cd ${pos_working_dir}/test/functional_requirements/; ./run_srm_tests.py -f ${target_fabric_ip}