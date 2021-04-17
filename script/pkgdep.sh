#!/bin/sh
# Please run this script as root.

SYSTEM=`uname -s`
POS_ROOT=`pwd`/../
SPDK_DIR=spdk
SPDK_ROOT=${POS_ROOT}/lib/${SPDK_DIR}/

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
    # for SPDK
    $SPDK_ROOT/scripts/pkgdep.sh
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

else
    echo "pkgdep: unknown system type."
	exit 1
fi
