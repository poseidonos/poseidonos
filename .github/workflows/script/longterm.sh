target_fabric_ip=127.0.0.1
pos_working_dir="$1" 

cd ${pos_working_dir}/test/regression; sudo ./long_term_ci_test.sh -f ${target_fabric_ip} -t 14400 -i 2 -s 50 -c false

# test iteration for a night : 1~2
# test iteration for a weekend : 10

# total size for vm : 100
# total size for pm : 12500