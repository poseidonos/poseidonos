#!/bin/sh
# Please run this script as root.

SYSTEM=`uname -s`
POS_ROOT=$(readlink -f $(dirname $0))/..

${POS_ROOT}/script/install_go.sh

if [ -f /etc/debian_version ]; then
    # Includes Ubuntu, Debian
    apt-get install -y gcc g++ make git 
    # Additional dependencies for NVMe over Fabrics
    apt-get install -y libibverbs-dev librdmacm-dev
    # Additional dependencies for JSON
    apt-get install -y libjsoncpp-dev
    # Additional dependencies for Userspace PCI Access
    apt-get install -y libpciaccess-dev
    # Additional dependencies for SPDK CLI
    apt-get install -y "python-configshell*"
    # Additional dependencies for DPDK
    apt-get install -y libnuma-dev uuid-dev
    # Additional RDMA
    apt-get install -y libnuma-dev uuid-dev ibverbs-utils perftest
    # Additional dependencies for iBOF OS
    apt-get install -y python3 python-pip
    apt-get install -y iperf
    # for ibof logging
    apt-get install -y cmake libsystemd-dev
    # for json
    apt-get install -y jq rapidjson-dev
    # for python3
    apt-get install -y python3
    # for psutil
    apt-get install -y python3-psutil
    # Compiler cache
    apt-get install -y ccache
    # For Vpp
    apt-get install -y libmnl-dev
    # For filebench
    apt install -y automake autoconf libtool bison flex python3-paramiko
    # For xfs
    apt install -y xfsprogs
    # for clang-format
    apt install -y clang-format
    # for nvdimm
    apt install -y libpmemblk-dev
    # for UT build
    apt install -y meson
    # for python3-tz (timezone)
    apt install -y python3-tz
    # for gdb
    apt install -y gdb
    # rpc client for cpp for invoke spdk rpc
    apt install -y libjsonrpccpp-dev libjsoncpp-dev
    # for grpc
    apt install -y libc-ares-dev pkg-config cmake
    # for pigz (trigger core dump, load dump)
    apt install -y pigz
    # for cli man page
    apt-get install -y mandoc
    # for yaml cpp
    apt-get install -y libyaml-cpp-dev
    # for numa test
    apt-get install -y numactl
    # for tbb
    apt install -y libtbb-dev
    # for nvme modules
    name=$(uname -r); apt-get install -y linux-modules-extra-${name}
    # for tcmalloc
    apt install -y libgoogle-perftools-dev


else
    echo "pkgdep: unknown system type."
	exit 1
fi
