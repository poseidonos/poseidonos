target_fabric_ip=127.0.0.1
pos_working_dir="$1" #추후 수정
config_dir=/etc/pos/pos.conf

# back up config file
cp ${config_dir} /etc/pos/pos.conf_bak

# test for linear flow control
jq '.flow_control.enable = true | .flow_control.use_default = false | .flow_control.strategy = \"linear\"' ${config_dir} > ./temp.conf && mv ./temp.conf ${config_dir}
cd ${pos_working_dir}/test/regression/; ./npor_ci_test.sh -f ${target_fabric_ip} -i 1

# test for state flow control
jq '.flow_control.strategy = \"state\"' ${config_dir} > ./temp.conf && mv ./temp.conf ${config_dir}
cd ${pos_working_dir}/test/regression/; ./npor_ci_test.sh -f ${target_fabric_ip} -i 1

# test for disable flow control
jq '.flow_control.enable = false' ${config_dir} > ./temp.conf && mv ./temp.conf ${config_dir}
cd ${pos_working_dir}/test/regression/; ./npor_ci_test.sh -f ${target_fabric_ip} -i 1

# test for qos disable
jq '.fe_qos.enable = true' ${config_dir} > ./temp.conf && mv ./temp.conf ${config_dir}
cd ${pos_working_dir}/test/functional_requirements/qos; sudo ./CREATE_QOS_VOLUME_POLICY_BASIC.py ${target_fabric_ip}
jq '.fe_qos.enable = false' ${config_dir} > ./temp.conf && mv ./temp.conf ${config_dir}

# roll back config file
mv /etc/pos/pos.conf_bak ${config_dir}