#
#!/bin/bash
#
# Note: need to run poseidonos/script/pkgdep.sh for SPDK/DPDK prerequistes

DPDK_DIR=dpdk-20.08
DPDK_SOURCE=${DPDK_DIR}.tar.gz
GPERF_SOURCE=gperftools-2.7.tar.gz
AIR_SOURCE=air
SPDK_DIR=spdk-20.10
SPDK_SOURCE=v20.10.tar.gz
GO_SOURCE=go1.14.4.linux-amd64.tar.gz
GTEST_SOURCE=release-1.10.0.tar.gz
GTEST_TARGET_LIB_DIR=/usr/local/lib
SPDLOG_SOURCE=spdlog-1.4.2
FILEBENCH_DIR=filebench-master

IS_SPDK_DBG_BUILD=n
KERNEL_VER="`uname -r`"
VM_KERNEL_VER="5.3.0-19-generic" 
IS_VM=0
if [ ${KERNEL_VER} = ${VM_KERNEL_VER} ]; then
    AIR_SOURCE_CI=${AIR_SOURCE}.vm
    DPDK_SOURCE_CI=${DPDK_SOURCE}.vm
    SPDLOG_SOURCE_CI=${SPDLOG_SOURCE}.vm
    FIO_SOURCE=fio-fio-3.12
    FIO_TARBALL=fio-3.12.tar.gz
    IS_VM=1
else
    AIR_SOURCE_CI=${AIR_SOURCE}
    DPDK_SOURCE_CI=${DPDK_SOURCE}
    SPDLOG_SOURCE_CI=${SPDLOG_SOURCE}
    FIO_SOURCE=fio-fio-3.1
    FIO_TARBALL=fio-3.1.tar.gz
fi

ROOT_DIR=$(readlink -f $(dirname $0))/
cd $ROOT_DIR

set_red(){
    echo -e "\033[31m"
}
set_green(){
    echo -e "\033[32m"
}

set_white(){
    echo -e "\033[0m"
}

log_normal(){
    set_green && echo $1 && set_white
}

log_error(){
    set_red && echo $1 && set_white
}

build_dpdk(){
    if [ ! -d "$DPDK_DIR" ]; then
	log_normal "[Download DPDK Source]"
	wget http://static.dpdk.org/rel/${DPDK_SOURCE}
	tar xzvf ${DPDK_SOURCE}
	ln -sf ${DPDK_DIR} dpdk
	cd ..
	patch -p0 < lib/dpdk-20.08.patch
	cd lib
    fi

    log_normal "[Build $DPDK_SOURCE ]"

    cd dpdk
    make install MAKE_PAUSE=n T=x86_64-native-linuxapp-gcc EXTRA_CFLAGS="-fPIC -Wno-address-of-packed-member" DESTDIR=. -j 4
    ret=$?
    cd -
    if [ $ret = 0 ]; then
	log_normal "[Build $DPDK_SOURCE ].. Done"
    else
	log_error "[Build $DPDK_SOURCE ].. Error"
    fi
}

build_fio(){
    if [ ! -d "$FIO_SOURCE" ]; then
        log_normal "[Download FIO Source]"
        wget https://github.com/axboe/fio/archive/refs/tags/$FIO_TARBALL
        tar xzvf $FIO_TARBALL
        patch -p0 < $FIO_SOURCE.patch
    fi
    
    log_normal "[Build $FIO_SOURCE ]"
    cd ${FIO_SOURCE}
    if [ ! -f "fio" ]; then
	make -j 4
	ret=$?
	if [ $ret = 0 ]; then
	    log_normal "[Build $FIO_SOURCE ].. Done"
	else
	    log_error "[Build $FIO_SOURCE ].. Error"
	fi
    fi
    cd -
}

build_gperf(){
    if [ ! -d "gperftools-2.7" ]; then
        log_normal "[Download GPERF Source]"
        wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.7/gperftools-2.7.tar.gz --no-check-certificate
        tar xzvf $GPERF_SOURCE
    fi

    log_normal "[Build $GPERF_SOURCE ]"
    if [ ! -d "gperftools-2.7" ]; then
        log_normal "extract GPERF"
        tar -xvf $GPERF_SOURCE 1>/dev/null
        log_normal "extract GPERF.. done"
    fi

    cd gperftools-2.7/
    if [ ! -f ".libs/libtcmalloc.a" ]; then
        ./configure && make -j && make install && ldconfig
        ret=$?
        if [ $ret = 0 ]; then
            log_normal "[Build $GPERF_SOURCE ].. Done"
        else
            log_error "[Build $GPERF_SOURCE ].. Error"
        fi
    fi
    cd -
}

build_spdk(){
    if [ ! -d ${SPDK_DIR} ]; then
	log_normal "[Download SPDK Source]"
        wget https://github.com/spdk/spdk/archive/refs/tags/${SPDK_SOURCE} --no-check-certificate
        tar xzvf $SPDK_SOURCE
	ln -sf ${SPDK_DIR} spdk
	cd spdk
	patch -p0 < ../spdk-20.10.patch
	cd ../
    fi

    log_normal "[Build $SPDK_SOURCE ]"

    cd spdk

    SPDK_CONFIG=" --with-dpdk=$ROOT_DIR/${DPDK_DIR}/ --with-rdma --with-fio=$ROOT_DIR/${FIO_SOURCE} --with-pos --without-isal --without-vhost --with-pmdk"
    if [ $IS_SPDK_DBG_BUILD = "y" ]; then
        SPDK_CONFIG=$SPDK_CONFIG" --enable-debug"
    fi

    if [ ! -f "mk/config.mk" ]; then
	./configure ${SPDK_CONFIG}
    else
        CONFIG_DBG=$(awk -F"=" '/CONFIG_DEBUG?/ {print $2}' mk/config.mk)
        if [ $IS_SPDK_DBG_BUILD != $CONFIG_DBG ]; then
	    ./configure ${SPDK_CONFIG}
        fi
    fi
    make -j 4
    ret=$?
    cp build/fio/spdk_nvme examples/nvme/fio_plugin/fio_plugin
    cd -
    if [ $ret = 0 ]; then
	log_normal "[Build $SPDK_SOURCE ].. Done"
    else
	log_error "[Build $SPDK_SOURCE ].. Error"
    fi
}

set_spdk_rel_build(){
    IS_SPDK_DBG_BUILD=n
}

build_gtest(){
    if [ ! -d "googletest-release-1.10.0" ]; then
        log_normal "[Download GTEST Source]"
        wget https://github.com/google/googletest/archive/refs/tags/release-1.10.0.tar.gz --no-check-certificate
        tar xzvf $GTEST_SOURCE
    fi

    GTEST_SOURCE_DIR=googletest-release-1.10.0
    GTEST_DIR=googletest
    GMOCK_DIR=googlemock
    GTEST_TARGET_INC_DIR=/usr/local/include
    cd ${GTEST_SOURCE_DIR}
    # https://github.com/google/googletest/tree/master/googletest
    g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
	-isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} \
    	-pthread -c ${GTEST_DIR}/src/gtest-all.cc
    # https://github.com/google/googletest/tree/master/googlemock
    g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
	-isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} \
    	-pthread -c ${GMOCK_DIR}/src/gmock-all.cc
    # https://github.com/google/googletest/tree/master/googlemock
    g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} \
	-isystem ${GMOCK_DIR}/include -I${GMOCK_DIR} \
    	-pthread -c ${GMOCK_DIR}/src/gmock_main.cc
    ar -rv ${GTEST_TARGET_LIB_DIR}/libgtest.a gtest-all.o gmock_main.o
    ar -rv ${GTEST_TARGET_LIB_DIR}/libgmock.a gtest-all.o gmock-all.o
    ar -rv ${GTEST_TARGET_LIB_DIR}/libgmock_main.a gtest-all.o gmock-all.o gmock_main.o
    cp -r ${GTEST_DIR}/include/gtest ${GTEST_TARGET_INC_DIR}
    cp -r ${GMOCK_DIR}/include/gmock ${GTEST_TARGET_INC_DIR}
    echo "Google test lib. & header files have been copied to ${GTEST_TARGET_LIB_DIR} & ${GTEST_TARGET_INC_DIR}"

    ret=$?
    cd -
    if [ $ret = 0 ]; then
	log_normal "[Build $GTEST_SOURCE ].. Done"
    else
	log_error "[Build $GTEST_SOURCE ].. Error"
    fi
}

build_air(){
    if [ -d "${AIR_SOURCE}" ]; then
        cd ${AIR_SOURCE}
        log_normal "[Build $AIR_SOURCE]"
        make release
        ret=$?
        if [ $ret = 0 ]; then
            log_normal "[Build $AIR_SOURCE].. Done"
        else
            log_normal "[Build $AIR_SOURCE].. Error"
        fi
        cd -
    else
        log_normal "No dir: $AIR_SOURCE"
    fi
}

build_air_ci(){
    if [ ${IS_VM} -eq 1 ]; then
	log_normal "[Build $AIR_SOURCE]"
	rm -rf ${AIR_SOURCE}
	cp -rf ${AIR_SOURCE_CI} ${AIR_SOURCE}
	log_normal "[Build $AIR_SOURCE].. Done"
    else
	build_air
    fi
}

build_spdlog(){
    if [ ! -d "${SPDLOG_SOURCE}" ]; then
        log_normal "[Download SPDLOG Source]"
        wget https://github.com/gabime/spdlog/archive/refs/tags/v1.4.2.tar.gz --no-check-certificate
        tar xzvf v1.4.2.tar.gz
        patch -p0 < spdlog-1.4.2.patch
    fi

    if [ -d "${SPDLOG_SOURCE}" ]; then
        cd ${SPDLOG_SOURCE}
        log_normal "[Build $SPDLOG_SOURCE]"
	cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=. -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON

	cmake --build _builds --target install
        cd -
    else
        log_normal "No dir: $SPDLOG_SOURCE"
    fi
}

build_spdlog_ci(){
    if [ ${IS_VM} -eq 1 ]; then
	log_normal "[Copy spdlog lib]"
	cp -rf ${SPDLOG_SOURCE_CI}/libspdlog.a /usr/local/lib/
	log_normal "[Copy spdlog lib].. Done"
    else
	build_spdlog
    fi
}

build_go() {
    log_normal "[SETUP $GO_SOURCE ]"
    ${ROOT_DIR}/../script/install_go.sh
}

build_filebench() {
    log_normal "[SETUP Filebench ]"
    if [ ! -d "${FILEBENCH_DIR}" ]; then
        log_normal "extract filebench"
        unzip ${FILEBENCH_DIR}.zip 1>/dev/null
        log_normal "extract filebench.. done"
    fi

    cd ${FILEBENCH_DIR}
    libtoolize
    aclocal
    autoheader
    automake --add-missing
    autoconf
    ./configure
    make
    make install
    cd -
    log_normal "[SETUP Filebench.. done]"
}

get_nlohmann_json() {
    mkdir -p nlohmann
    wget -O nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.7.3/json.hpp --no-check-certificate
}


clean_filebench() {
    log_normal "clean Filebench"
    rm -rf ${FILEBENCH_DIR}
    log_normal "clean Filebench.. done"
}


clean_fio()
{
    log_normal "clean FIO"
    if [ -d "${FIO_SOURCE}" ]; then
	cd ${FIO_SOURCE}
	make clean
	cd -
    fi
    log_normal "clean FIO.. done"
}

clean_spdk()
{
    log_normal "clean SPDK"
    if [ -d ""spdk"" ]; then
	cd spdk
	make clean
	rm mk/config.mk
	cd -
    fi
    log_normal "clean SPDK.. done"
}

clean_dpdk()
{
    log_normal "clean DPDK"
    if [ -d "dpdk" ]; then
	cd dpdk
	rm -rf x86_64-native-linuxapp-gcc
	rm -rf lib/modules
	rm -rf lib/*.a
	cd -
    fi
    log_normal "clean DPDK.. done"
}

clean_air()
{
    log_normal "clean AIR"
    if [ -d "${AIR_SOURCE}" ]; then
        cd ${AIR_SOURCE}
        make clean
        cd -
    fi
    log_normal "clean AIR.. done"
}

clean_spdlog()
{
    log_normal "clean SPDLOG"
    if [ -d "${SPDLOG_SOURCE}" ]; then
        cd ${SPDLOG_SOURCE}
	rm -rf _builds
	rm -rf ./lib/*.a
        cd -
    fi
    log_normal "clean SPDLOG.. done"
}

clean_gtest()
{
    log_normal "clean GTEST"
    if [ -d "googletest-release-1.10.x" ]; then
	cd googletest-release-1.10.x
	make clean && make distclean && rm configure
	rm ${GTEST_TARGET_LIB_DIR}/libgtest.a
	rm ${GTEST_TARGET_LIB_DIR}/libgmock.a
	rm ${GTEST_TARGET_LIB_DIR}/libgmock_main.a
	cd -
    fi
    log_normal "clean GTEST.. done"
}

clean_go()
{
    log_normal "clean GO"
    if [ -d "go" ]; then
	rm -rf ./go
    fi
    log_normal "clean GO.. done"
}

case "$1" in
    all)
	set -e
	get_nlohmann_json
	build_dpdk
	build_fio
	build_gperf
	build_air
	set_spdk_rel_build
	build_spdk
	build_go
	build_gtest
	build_spdlog
	;;
    ci)
	set -e
	build_dpdk_ci
	build_fio
	build_gperf
	build_air_ci
	build_spdk
	build_spdlog_ci
	build_go
	build_filebench
	;;
    perf_ci)
	set -e
	set_spdk_rel_build
	build_dpdk_ci
	build_fio
	build_gperf
	build_air_ci
	build_spdk
	build_spdlog_ci
	build_go
	build_filebench
	;;
    dpdk)
	set -e
	build_dpdk
	;;
    spdk)
	set -e
	build_fio
	build_gperf
	build_air
	build_spdk
	;;
    spdk_rel)
	set -e
	set_spdk_rel_build
	build_fio
	build_gperf
	build_air
	build_spdk
	;;
    gtest)
	set -e
	build_gtest
	;;
    air)
	set -e
	build_air
	;;
    spdlog)
	set -e
	build_spdlog
	;;
    go)
	set -e
	build_go
	;;
    filebench)
	set -e
	build_filebench
	;;
    clean_ci)
	clean_fio
	clean_spdk
	clean_dpdk
	clean_air
	clean_spdlog
	clean_go
	clean_filebench
	;;
    clean)
	clean_fio
	clean_spdk
	clean_dpdk
	clean_air
	clean_spdlog
	clean_gtest
	clean_go
	;;

    *)
	echo "Usage: build_ibof_lib.sh {all|dpdk|spdk|spdk_rel|gtest|air|spdlog|clean|go}"
	exit 1
	;;
esac
exit 0
