#
#!/bin/bash
#
# Note: need to run ibofos/script/pkgdep.sh for SPDK/DPDK prerequistes

DPDK_SOURCE=dpdk-stable-19.08.2
GPERF_SOURCE=gperftools-2.7.tar.gz
AIR_SOURCE=air
SPDK_SOURCE=v19.10.tar.gz
GO_SOURCE=go1.14.4.linux-amd64.tar.gz
GTEST_SOURCE=release-1.10.0.tar.gz
GTEST_TARGET_LIB_DIR=/usr/local/lib
SPDLOG_SOURCE=spdlog-1.4.2
FILEBENCH_DIR=filebench-master

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
    if [ ! -d "$DPDK_SOURCE" ]; then
        log_normal "[Download DPDK Source]"
        wget http://static.dpdk.org/rel/dpdk-19.08.2.tar.gz
        tar xzvf dpdk-19.08.2.tar.gz
        patch -p0 < dpdk-19.08.2.patch
    fi

    log_normal "[Build DPDK]"
    cd $DPDK_SOURCE
    make install T=x86_64-native-linuxapp-gcc EXTRA_CFLAGS="-fPIC -Wno-address-of-packed-member" DESTDIR=. -j
    ret=$?
    cd -
    if [ $ret = 0 ]; then
        log_normal "[Build DPDK].. Done"
    else
        log_error "[Build DPDK].. Error"
    fi
}

build_fio(){
    if [ ! -d "$FIO_SOURCE" ]; then
        log_normal "[Download FIO Source]"
        wget https://github.com/axboe/fio/archive/refs/tags/$FIO_TARBALL
        tar xzvf $FIO_TARBALL
        patch -p0 < $FIO_SOURCE.patch
    fi

    log_normal "[Build FIO]"
    cd ${FIO_SOURCE}
	if [ ! -f "fio" ]; then
		make -j
		ret=$?
		if [ $ret = 0 ]; then
			log_normal "[Build FIO].. Done"
		else
			log_error "[Build FIO].. Error"
		fi
	fi
	cd -
}

build_dpdk_ci(){
	if [ ${IS_VM} -eq 1 ]; then
		log_normal "[Build $DPDK_SOURCE]"
		rm -rf ${DPDK_SOURCE}
		cp -rf ${DPDK_SOURCE_CI} ${DPDK_SOURCE}
		log_normal "[Build $DPDK_SOURCE].. Done"
	else
		build_dpdk
	fi
}

build_gperf(){
    if [ ! -d "gperftools-2.7" ]; then
        log_normal "[Download GPERF Source]"
        wget https://github.com/gperftools/gperftools/releases/download/gperftools-2.7/gperftools-2.7.tar.gz --no-check-certificate
        tar xzvf $GPERF_SOURCE
    fi
 
    log_normal "[Build GPERF]"
    cd gperftools-2.7/
    if [ ! -f "./lib/libtcmalloc.a" ]; then
        ./configure && make -j && make install && ldconfig
        ret=$?
        if [ $ret = 0 ]; then
            log_normal "[Build GPERF].. Done"
        else
            log_error "[Build GPERF].. Error"
        fi
    fi
    cd -
}

build_spdk(){
	if [ ! -d "spdk-19.10" ]; then
    log_normal "[Download SPDK Source]"
        wget https://github.com/spdk/spdk/archive/refs/tags/v19.10.tar.gz --no-check-certificate
        tar xzvf $SPDK_SOURCE
        patch -p0 < spdk-19.10.patch
	fi

	log_normal "[Build SPDK]"
	cd spdk-19.10
	SPDK_CONFIG="--enable-debug --with-dpdk=$ROOT_DIR/dpdk-stable-19.08.2/ --with-rdma --with-fio=$ROOT_DIR/${FIO_SOURCE} --with-ibof --without-isal"
	if [ ! -f "mk/config.mk" ]; then
		./configure ${SPDK_CONFIG}
	fi
	make -j
	ret=$?
	cd -
	if [ $ret = 0 ]; then
		log_normal "[Build SPDK].. Done"
	else
		log_error "[Build SPDK].. Error"
	fi
}

build_gtest(){
	if [ ! -d "googletest-release-1.10.0" ]; then
        log_normal "[Download GTEST Source]"
        wget https://github.com/google/googletest/archive/refs/tags/release-1.10.0.tar.gz --no-check-certificate
        tar xzvf $GTEST_SOURCE
	fi

	log_normal "[Build GTEST]"
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
		log_normal "[Build GTEST].. Done"
	else
		log_error "[Build GTEST].. Error"
	fi
}

build_nvme_cli(){
	log_normal "[Build spdk nvme-cli-1.8.1 ]"
	cd nvme-cli-1.8.1
	if [ ! -f "nvme" ]; then
		make -j 4
		ret=$?
		if [ $ret = 0 ]; then
			log_normal "[Build spdk nvme-cli-1.8.1 ].. Done"
		else
			log_error "[Build spdk nvme-cli-1.8.1 ].. Error"
		fi
	fi
	cd -
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

    cd ${SPDLOG_SOURCE}
    log_normal "[Build SPDLOG]"
    cmake -H. -B_builds -DCMAKE_INSTALL_PREFIX=. -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    cmake --build _builds --target install
    cd -
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
	if [ ! -d "go" ]; then
        log_normal "download GO"
        wget "https://dl.google.com/go/${GO_SOURCE}"
        log_normal "download GO.. done"
		
		log_normal "extract GO"
		tar -xvf $GO_SOURCE 1>/dev/null
		log_normal "extract GO.. done"
	fi
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

clean_filebench() {
    log_normal "clean Filebench"
    rm -rf ${FILEBENCH_DIR}
	log_normal "clean Filebench.. done"
}

get_nlohmann_json() {
    mkdir -p nlohmann
    wget -O nlohmann/json.hpp https://github.com/nlohmann/json/releases/download/v3.7.3/json.hpp --no-check-certificate
}

clean_nvme()
{
	log_normal "clean nvme-cli-1.8.1"
	if [ -d "nvme-cli-1.8.1" ]; then
		cd nvme-cli-1.8.1
		make clean
		cd -
	fi
	log_normal "clean nvme-cli.. done"
}

clean_spdk()
{
	log_normal "clean SPDK"
	if [ -d ""spdk-19.10"" ]; then
		cd spdk-19.10
		make clean
		rm mk/config.mk
		cd -
	fi
	log_normal "clean SPDK.. done"
}

clean_dpdk()
{
	log_normal "clean DPDK"
	if [ -d "dpdk-stable-19.08.2" ]; then
		cd dpdk-stable-19.08.2
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
    build_spdk
    build_go
    build_gtest
    build_spdlog
    ;;
ci)
    set -e
    get_nlohmann_json
    build_dpdk_ci
    build_fio
    build_gperf
    build_air_ci
    build_spdk
    build_spdlog_ci
    build_go
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
    ;;
clean_ci)
	clean_nvme
	clean_spdk
	clean_dpdk
	clean_air
	clean_spdlog
	clean_go
    clean_filebench
	;;	
clean)
	clean_nvme
	clean_spdk
	clean_dpdk
	clean_air
	clean_spdlog
	clean_gtest
	clean_go
    clean_filebench
	;;

*)
	echo "Usage: build_ibof_lib.sh {all|ci|dpdk|spdk|gtest|air|spdlog|clean|clean_ci|go|filebench}"
	exit 1
	;;
esac
exit 0
