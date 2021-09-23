#!/bin/bash

GRPC_REPO="https://10.227.30.174:7990/scm/ibof/grpc.git"
INSTALL_DIR="/usr/local"
BUILD_JOBS=4
FORCE_INSTALL=FALSE
BUILD_INTERNAL=FALSE
INSTALLED=FALSE

check_installation() {
	if [ -e "${INSTALL_DIR}/bin/grpc_cpp_plugin" ]
	then
		INSTALLED=TRUE
	fi
}

install_re2()
{
	mkdir -p "third_party/re2/cmake/build"
	pushd "third_party/re2/cmake/build"
	cmake ../.. \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_POSITION_INDEPENDENT_CODE=TRUE \
		-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
	make -j ${BUILD_JOBS}
	make install
	popd
}

install_protobuf()
{
	mkdir -p third_party/protobuf/cmake/build
	pushd third_party/protobuf/cmake/build
	cmake .. \
		-Dprotobuf_BUILD_TESTS=OFF \
		-DCMAKE_POSITION_INDEPENDENT_CODE=TRUE \
		-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
	make -j ${BUILD_JOBS}
	make install
	popd
}

install_grpc()
{
	mkdir -p cmake/build
	pushd cmake/build
	cmake ../..	\
		-DgRPC_INSTALL=ON	\
		-DgRPC_BUILD_TESTS=OFF	\
		-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}	\
		-DCMAKE_POSITION_INDEPENDENT_CODE=TRUE  \
		-DgRPC_BUILD_GRPC_CSHARP_PLUGIN=OFF	\
		-DgRPC_BUILD_GRPC_NODE_PLUGIN=OFF	\
		-DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=OFF	\
		-DgRPC_BUILD_GRPC_PHP_PLUGIN=OFF	\
		-DgRPC_BUILD_GRPC_PYTHON_PLUGIN=OFF	\
		-DgRPC_BUILD_GRPC_RUBY_PLUGIN=OFF	\
		-DgRPC_BUILD_CSHARP_EXT=OFF	\
		-DgRPC_ABSL_PROVIDER=package	\
		-DgRPC_CARES_PROVIDER=package	\
		-DgRPC_RE2_PROVIDER=package	\
		-DgRPC_SSL_PROVIDER=package	\
		-DgRPC_ZLIB_PROVIDER=package \
		-DCMAKE_BUILD_TYPE=Release \
		-DgRPC_PROTOBUF_PROVIDER=package

	make -j ${BUILD_JOBS}
	make install
	popd
}

install_abseil()
{
	mkdir -p third_party/abseil-cpp/cmake/build
	pushd third_party/abseil-cpp/cmake/build
	cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
		-DCMAKE_POSITION_INDEPENDENT_CODE=TRUE \
		../..
	make -j ${BUILD_JOBS}
	make install
	popd
}

main()
{
	echo "GRPC installation start."
	if [ ${FORCE_INSTALL} == FALSE ]
	then
		check_installation
		if [ ${INSTALLED} == TRUE ]
		then
			echo "GRPC installation stopped. already installed."
			exit 0
		fi
	fi
	if [ ${BUILD_INTERNAL} == FALSE ]
	then
		GRPC_REPO="-b v1.38.0 https://github.com/grpc/grpc.git"
	fi

	rm -rf grpc
	git clone ${GRPC_REPO}
	cd grpc
	if [ ${BUILD_INTERNAL} == FALSE ]
	then
		echo "Cleaning up existing dirs in grpc/third_party/(re2, abseil-cpp, protobuf)"
		rm -rf third_party/re2 third_party/abseil-cpp third_party/protobuf

		echo "Checking out GRPC submodules..."
		git submodule update --init
	fi
	
	install_re2
	install_protobuf
	install_abseil
	install_grpc

	echo "GRPC installation success"
}

help()
{
	echo "Usage: ./build_grpc.sh [OPTION]"
	echo " -f	force install"

}

while getopts "fhi" opt
do
	case $opt in
		f)
			FORCE_INSTALL=TRUE
			;;
		h)
			help
			exit 0;
			;;
		i)
			BUILD_INTERNAL=TRUE
	esac
done

main
