echo "nvmf_create_transport"
./rpc.py nvmf_create_transport -t TCP -n 4096 -b 64

echo "bdev_nvme_attach_controller"
./rpc.py bdev_nvme_attach_controller -t PCIe -b Nvme1 -a 0000:1c:00.0
./rpc.py bdev_nvme_attach_controller -t PCIe -b Nvme2 -a 0000:1d:00.0
./rpc.py bdev_nvme_attach_controller -t PCIe -b Nvme3 -a 0000:5e:00.0
./rpc.py bdev_nvme_attach_controller -t PCIe -b Nvme4 -a 0000:5f:00.0
./rpc.py bdev_nvme_attach_controller -t PCIe -b Nvme5 -a 0000:da:00.0
./rpc.py bdev_nvme_attach_controller -t PCIe -b Nvme6 -a 0000:db:00.0
./rpc.py bdev_nvme_attach_controller -t PCIe -b Nvme7 -a 0000:de:00.0
./rpc.py bdev_nvme_attach_controller -t PCIe -b Nvme8 -a 0000:df:00.0

echo "nvmf_create_subsystem"
./rpc.py nvmf_create_subsystem nqn.2019-10.spdk:subsystem1 -a -s SPDK0001 -d SPDK_VOL1
./rpc.py nvmf_create_subsystem nqn.2019-10.spdk:subsystem2 -a -s SPDK0002 -d SPDK_VOL2
./rpc.py nvmf_create_subsystem nqn.2019-10.spdk:subsystem3 -a -s SPDK0003 -d SPDK_VOL3
./rpc.py nvmf_create_subsystem nqn.2019-10.spdk:subsystem4 -a -s SPDK0004 -d SPDK_VOL4
./rpc.py nvmf_create_subsystem nqn.2019-10.spdk:subsystem5 -a -s SPDK0005 -d SPDK_VOL5
./rpc.py nvmf_create_subsystem nqn.2019-10.spdk:subsystem6 -a -s SPDK0006 -d SPDK_VOL6
./rpc.py nvmf_create_subsystem nqn.2019-10.spdk:subsystem7 -a -s SPDK0007 -d SPDK_VOL7
./rpc.py nvmf_create_subsystem nqn.2019-10.spdk:subsystem8 -a -s SPDK0008 -d SPDK_VOL8

echo "nvmf_subsystem_add_ns"
./rpc.py nvmf_subsystem_add_ns nqn.2019-10.spdk:subsystem1 Nvme1n1
./rpc.py nvmf_subsystem_add_ns nqn.2019-10.spdk:subsystem2 Nvme2n1
./rpc.py nvmf_subsystem_add_ns nqn.2019-10.spdk:subsystem3 Nvme3n1
./rpc.py nvmf_subsystem_add_ns nqn.2019-10.spdk:subsystem4 Nvme4n1
./rpc.py nvmf_subsystem_add_ns nqn.2019-10.spdk:subsystem5 Nvme5n1
./rpc.py nvmf_subsystem_add_ns nqn.2019-10.spdk:subsystem6 Nvme6n1
./rpc.py nvmf_subsystem_add_ns nqn.2019-10.spdk:subsystem7 Nvme7n1
./rpc.py nvmf_subsystem_add_ns nqn.2019-10.spdk:subsystem8 Nvme8n1

echo "nvmf_subsystem_add_listener"
./rpc.py nvmf_subsystem_add_listener nqn.2019-10.spdk:subsystem1 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_add_listener nqn.2019-10.spdk:subsystem2 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_add_listener nqn.2019-10.spdk:subsystem3 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_add_listener nqn.2019-10.spdk:subsystem4 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_add_listener nqn.2019-10.spdk:subsystem5 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_add_listener nqn.2019-10.spdk:subsystem6 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_add_listener nqn.2019-10.spdk:subsystem7 -t tcp -a 10.100.4.19 -s 1158
./rpc.py nvmf_subsystem_add_listener nqn.2019-10.spdk:subsystem8 -t tcp -a 10.100.4.19 -s 1158

