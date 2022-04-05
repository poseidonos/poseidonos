target_fabric_ip=127.0.0.1
pos_working_dir="$1" 

cd ${pos_working_dir}/test/regression/; sudo ./nvme_smart_log_ci_test.sh -f ${target_fabric_ip}