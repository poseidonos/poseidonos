#
#!/bin/bash
#
# Note: need to run ibofos/script/pkgdep.sh for SPDK/DPDK prerequistes

SPDK_SOURCE=v19.10.tar.gz
DPDK_SOURCE=dpdk-stable-19.08.2
AIR_SOURCE=air
SPDLOG_SOURCE=spdlog-1.4.2
GO_SOURCE=go1.14.4.linux-amd64.tar.gz

KERNEL_VER="`uname -r`"
VM_KERNEL_VER="5.3.0-19-generic" 
IS_VM=0
if [ ${KERNEL_VER} = ${VM_KERNEL_VER} ]; then
	AIR_SOURCE_CI=${AIR_SOURCE}.vm
	DPDK_SOURCE_CI=${DPDK_SOURCE}.vm
	SPDLOG_SOURCE_CI=${SPDLOG_SOURCE}.vm
	FIO_SOURCE=312
	IS_VM=1
else
	AIR_SOURCE_CI=${AIR_SOURCE}
	DPDK_SOURCE_CI=${DPDK_SOURCE}
	SPDLOG_SOURCE_CI=${SPDLOG_SOURCE}
	FIO_SOURCE=31
fi

GTEST_SOURCE=googletest-release-1.10.x.tar.gz
GPERF_SOURCE=gperftools-2.7.tar.gz
GTEST_TARGET_LIB_DIR=/usr/local/lib

FILEBENCH_DIR=filebench-master

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
	log_normal "[Build $DPDK_SOURCE ]"

	cd dpdk-stable-19.08.2
	make install T=x86_64-native-linuxapp-gcc EXTRA_CFLAGS="-fPIC -Wno-address-of-packed-member" DESTDIR=. -j 4
	ret=$?
	cd -
	if [ $ret = 0 ]; then
		log_normal "[Build $DPDK_SOURCE ].. Done"
	else
		log_error "[Build $DPDK_SOURCE ].. Error"
	fi
}

build_dpdk_ci(){
	if [ ${IS_VM} -eq 1 ]; then
		log_normal "[Build $DPDK_SOURCE ]"
		rm -rf ${DPDK_SOURCE}
		cp -rf ${DPDK_SOURCE_CI} ${DPDK_SOURCE}
		log_normal "[Build $DPDK_SOURCE ].. Done"
	else
		build_dpdk
	fi
}

build_gperf(){
    log_normal "[Build $GPERF_SOURCE ]"
    if [ ! -d "gperftools-2.7" ]; then
        log_normal "extract GPERF"
        tar -xvf $GPERF_SOURCE 1>/dev/null
        log_normal "extract GPERF.. done"
    fi
 
    cd gperftools-2.7/
    if [ ! -f "./lib/libtcmalloc.a" ]; then
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
	log_normal "[Build $SPDK_SOURCE ]"
	if [ ! -d "spdk-19.10" ]; then
		log_normal "extract SPDK"
		tar -xvf $SPDK_SOURCE 1>/dev/null
		log_normal "extract SPDK.. done"
	fi

	cd spdk-19.10
	SPDK_CONFIG="--enable-debug --with-dpdk=$ROOT_DIR/dpdk-stable-19.08.2/ --with-rdma --with-ibof --without-isal"
	if [ ! -f "mk/config.mk" ]; then
		./configure ${SPDK_CONFIG}
	fi
	make -j 4
	ret=$?
	cp fio_plugin/bdev/fio_plugin_${FIO_SOURCE} examples/bdev/fio_plugin/fio_plugin
        cp fio_plugin/nvme/fio_plugin_${FIO_SOURCE} examples/nvme/fio_plugin/fio_plugin
	cd -
	if [ $ret = 0 ]; then
		log_normal "[Build $SPDK_SOURCE ].. Done"
	else
		log_error "[Build $SPDK_SOURCE ].. Error"
	fi
}

build_gtest(){
	log_normal "[Build $GTEST_SOURCE ]"
	if [ ! -d "googletest-release-1.10.x" ]; then
		log_normal "extrast gtest-1.10.x"
		tar -xvf $GTEST_SOURCE 1>/dev/null
		log_normal "extract gtest.. done"
	fi

	GTEST_SOURCE_DIR=googletest-release-1.10.x
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
	build_dpdk
	build_gperf
	build_air
	build_spdk
	#build_nvme_cli
	build_go
	build_gtest
	build_spdlog
    #build_filebench
	;;
ci)
	set -e
	build_dpdk_ci
	build_gperf
    build_air_ci
	build_spdk
	#build_nvme_cli
	build_spdlog_ci
	build_go
    #build_filebench
	;;
dpdk)
	set -e
	build_dpdk
	;;
spdk)
	set -e
	build_gperf
	build_air
	build_spdk
	#build_nvme_cli
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
    #build_filebench
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
