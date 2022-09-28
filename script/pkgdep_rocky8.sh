# !/bin/sh
#  Please run this script as root.

set -e # exit immediately on any fail

POS_ROOT=$(readlink -f $(dirname $0))/..

dnf install -y epel-release
crb enable

dnf -y install \
    gcc gcc-c++ `# g++` make git \
    rdma-core-devel `# libibverbs-dev librdmacm-dev` \
    jsoncpp-devel `# libjsoncpp-dev` \
    libpciaccess-devel `# libpciaccess-dev` \
    python3-configshell `# "python-configshell*"` \
    numactl-devel `# libnuma-dev` libuuid-devel `# uuid-dev` libibverbs-utils `# ibverbs-utils` perftest \
    python3 python3-pip \
    iperf \
    cmake systemd-devel `# libsystemd-dev` \
    jq rapidjson-devel `# rapidjson-dev` \
    python3-psutil \
    ccache \
    libmnl-devel `# libmnl-dev` \
    automake autoconf libtool bison flex python3-paramiko \
    xfsprogs \
    clang-tools-extra `# clang-format` \
    libpmemblk-devel `# libpmemblk-dev` \
    meson \
    python3-pytz `# python3-tz` \
    gdb \
    `# libjsonrpccpp-dev does not exist` \
    c-ares-devel `# libc-ares-dev` pkg-config cmake \
    pigz \
    mandoc \
    yaml-cpp-devel `# libyaml-cpp-dev` \
    numactl \
    tbb-devel `# libtbb-dev` \
    `# linux-modules-extra not needed` \
    google-perftools-devel `# libgoogle-perftools-dev` \
    rocksdb-devel `# librocksdb-dev` \
    `# libisal-dev does not exist` \
    `# libasan included in gcc?`

# Additional packages
dnf -y install patch wget tar openssl-devel

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

# jsoncpp-devel installed in different directory: link it
mkdir -p /usr/include/jsoncpp
ln -s /usr/include/json /usr/include/jsoncpp/json

# RHEL does not look for libraries in /usr/local/lib unlike Ubuntu
echo '/usr/local/lib' > /etc/ld.so.conf.d/pos-local-lib.conf
