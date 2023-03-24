#!/bin/sh
# Please run this script as root.

SYSTEM=`uname -s`
POS_ROOT=$(readlink -f $(dirname $0))/..

source /etc/os-release

if [ -f /etc/debian_version ]; then
    ${POS_ROOT}/script/install_go.sh

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
    apt-get install -y python3 python-pip python3-pip
    apt-get install -y iperf
    # for ibof logging
    apt-get install -y cmake libsystemd-dev
    # for json
    apt-get install -y jq rapidjson-dev
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
    # for rocksdb
    apt install -y librocksdb-dev
    # for markdownTable
    pip3 install py-markdown-table
    # for pyyaml
    pip3 install pyyaml
    # for isal
    apt install -y libisal-dev
    # for address sanitizer
    apt install -y libasan4
    apt install -y libasan4-dbg
    # for crc
    apt install -y libboost-dev
    apt install -y libboost-system-dev 
    apt install -y libboost-thread-dev 
    # for opentelemetry
    apt install -y libssl-dev
    # for unit test
    apt install -y lcov

    python -m pip install --upgrade pip
    python3 -m pip install --upgrade pip
    pip3 install scikit-build
    pip install scikit-build
    pip3 install cmake
    pip install cmake

elif echo "$ID $VERSION_ID" | grep -E -q 'centos 8|rocky 8'; then
    set -e # exit immediately on any fail

    POS_ROOT=$(readlink -f $(dirname $0))/..

    dnf install -y epel-release
    dnf config-manager --set-enabled powertools

    PKGS="gcc gcc-c++ make git" # g++ -> gcc-c++
    PKGS="${PKGS} rdma-core-devel" # libibverbs-dev, librdmacm-dev
    PKGS="${PKGS} jsoncpp-devel" # libjsoncpp-dev
    PKGS="${PKGS} libpciaccess-devel" # libpciaccess-dev
    PKGS="${PKGS} python3-configshell" # python-configshell*
    PKGS="${PKGS} numactl-devel libuuid-devel" # libnuma-dev -> numactl-devel, uuid-dev -> libuuid-devel
    PKGS="${PKGS} libibverbs-utils perftest" # ibverbs-utils -> libibverbs-utils
    PKGS="${PKGS} python3 python3-pip"
    PKGS="${PKGS} iperf"
    PKGS="${PKGS} cmake systemd-devel" # libsystemd-dev -> systemd-devel
    PKGS="${PKGS} jq rapidjson-devel" # rapidjson-dev -> rapidjson-devel
    PKGS="${PKGS} python3-psutil"
    PKGS="${PKGS} ccache"
    PKGS="${PKGS} libmnl-devel" # libmnl-dev
    PKGS="${PKGS} automake autoconf libtool bison flex"
    PKGS="${PKGS} python3-paramiko"
    PKGS="${PKGS} xfsprogs"
    PKGS="${PKGS} clang-tools-extra" # clang-format
    PKGS="${PKGS} libpmemblk-devel" # libpmemblk-dev
    PKGS="${PKGS} meson"
    PKGS="${PKGS} python3-pytz" # python3-tz
    PKGS="${PKGS} gdb"
    # libjsonrpccpp-dev will be installed manually
    PKGS="${PKGS} c-ares-devel pkg-config cmake" # libc-ares-dev -> c-ares-devel
    PKGS="${PKGS} pigz"
    PKGS="${PKGS} mandoc"
    PKGS="${PKGS} yaml-cpp-devel" # libyaml-cpp-dev
    PKGS="${PKGS} numactl"
    PKGS="${PKGS} tbb-devel" # libtbb-dev
    # nvme module already available by default
    PKGS="${PKGS} google-perftools-devel" # libgoogle-perftools-dev
    PKGS="${PKGS} rocksdb-devel" # librocksdb-dev
    # libisal-dev will be installed manually
    # libasan -> gcc

    # Additional packages
    PKGS="${PKGS} patch wget tar openssl-devel"

    dnf install -y ${PKGS}

    ${POS_ROOT}/script/install_go.sh

    # dir to build unsupported dependencies
    rm -rf ${POS_ROOT}/dep
    mkdir -p ${POS_ROOT}/dep

    # libsonrpccpp
    ## libargtable
    cd ${POS_ROOT}/dep
    wget 'http://prdownloads.sourceforge.net/argtable/argtable2-13.tar.gz'
    tar xf argtable2-13.tar.gz
    cd argtable2-13
    ./configure
    make -j 4
    make install

    ## catch
    cd /usr/include
    wget 'https://github.com/catchorg/Catch2/releases/download/v1.12.2/catch.hpp'

    ## hiredis
    cd ${POS_ROOT}/dep
    git clone 'https://github.com/redis/hiredis.git'
    cd hiredis
    mkdir build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr
    make -j 4
    make install

    ## other deps
    dnf -y install libcurl-devel libmicrohttpd-devel jsoncpp-devel cmake

    ## finally itself
    cd ${POS_ROOT}/dep
    git clone 'https://github.com/cinemast/libjson-rpc-cpp.git'
    cd libjson-rpc-cpp
    git checkout v1.0.0
    mkdir build
    cd build
    cmake ..
    make -j 4
    make install
    ldconfig

    # isa-l
    cd ${POS_ROOT}/dep
    dnf -y install nasm
    git clone 'https://github.com/intel/isa-l.git'
    cd isa-l
    ./autogen.sh
    ./configure
    make
    make install

    python3 -m pip install py-markdown-table pyyaml

    python -m pip install --upgrade pip
    python3 -m pip install --upgrade pip
    pip3 install scikit-build
    pip install scikit-build
    pip3 install cmake
    pip install cmake
    # jsoncpp-devel installed in different directory: link it
    mkdir -p /usr/include/jsoncpp
    ln -s /usr/include/json /usr/include/jsoncpp/json

    # RHEL does not look for libraries in /usr/local/lib unlike Ubuntu
    echo '/usr/local/lib' > /etc/ld.so.conf.d/pos-local-lib.conf
else
    echo "pkgdep: unknown system type."
    exit 1
fi
