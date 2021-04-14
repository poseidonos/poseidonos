echo "nvmf_subsystem_remove_listener"
./rpc.py nvmf_subsystem_remove_listener nqn.2019-10.spdk:subsystem1 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_remove_listener nqn.2019-10.spdk:subsystem2 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_remove_listener nqn.2019-10.spdk:subsystem3 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_remove_listener nqn.2019-10.spdk:subsystem4 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_remove_listener nqn.2019-10.spdk:subsystem5 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_remove_listener nqn.2019-10.spdk:subsystem6 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_remove_listener nqn.2019-10.spdk:subsystem7 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_remove_listener nqn.2019-10.spdk:subsystem8 -t tcp -a 10.100.4.19 -s 1158

echo "nvmf_subsystem_remove_ns"
./rpc.py nvmf_subsystem_remove_ns nqn.2019-10.spdk:subsystem1 1
./rpc.py nvmf_subsystem_remove_ns nqn.2019-10.spdk:subsystem2 1 
./rpc.py nvmf_subsystem_remove_ns nqn.2019-10.spdk:subsystem3 1 
./rpc.py nvmf_subsystem_remove_ns nqn.2019-10.spdk:subsystem4 1 
./rpc.py nvmf_subsystem_remove_ns nqn.2019-10.spdk:subsystem5 1 
./rpc.py nvmf_subsystem_remove_ns nqn.2019-10.spdk:subsystem6 1 
./rpc.py nvmf_subsystem_remove_ns nqn.2019-10.spdk:subsystem7 1 
./rpc.py nvmf_subsystem_remove_ns nqn.2019-10.spdk:subsystem8 1 

echo "nvmf_delete_subsystem"
./rpc.py nvmf_delete_subsystem nqn.2019-10.spdk:subsystem1
./rpc.py nvmf_delete_subsystem nqn.2019-10.spdk:subsystem2
./rpc.py nvmf_delete_subsystem nqn.2019-10.spdk:subsystem3
./rpc.py nvmf_delete_subsystem nqn.2019-10.spdk:subsystem4
./rpc.py nvmf_delete_subsystem nqn.2019-10.spdk:subsystem5
./rpc.py nvmf_delete_subsystem nqn.2019-10.spdk:subsystem6
./rpc.py nvmf_delete_subsystem nqn.2019-10.spdk:subsystem7
./rpc.py nvmf_delete_subsystem nqn.2019-10.spdk:subsystem8

echo "bdev_nvme_detach"
./rpc.py bdev_nvme_detach_controller Nvme1
./rpc.py bdev_nvme_detach_controller Nvme2
./rpc.py bdev_nvme_detach_controller Nvme3
./rpc.py bdev_nvme_detach_controller Nvme4
./rpc.py bdev_nvme_detach_controller Nvme5
./rpc.py bdev_nvme_detach_controller Nvme6
./rpc.py bdev_nvme_detach_controller Nvme7
./rpc.py bdev_nvme_detach_controller Nvme8

echo "spdk_kill_instance"
./rpc.py spdk_kill_instance SIGINT

