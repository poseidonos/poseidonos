#!/bin/bash

GRPC_REPO="http://10.227.30.174:7990/scm/ibof/grpc.git"
INSTALL_DIR="/usr/local"
BUILD_JOBS=4
FORCE_INSTALL=FALSE
INSTALLED=FALSE

check_installation() {
	if [[ -d "${INSTALL_DIR}/include/grpc"
		&& -d "${INSTALL_DIR}/include/absl" ]]
	then
		INSTALLED=TRUE
	fi
}

install_grpc()
{
	cd grpc
	mkdir -p cmake/build
	pushd cmake/build
	cmake -DgRPC_INSTALL=ON \
      -DgRPC_BUILD_TESTS=OFF \
      -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
      ../..
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

	git clone ${GRPC_REPO}
	cd grpc
	install_grpc
	install_abseil

	echo "GRPC installation success"
}

help()
{
	echo "Usage: ./build_grpc.sh [OPTION]"
	echo " -f	force install"

}

while getopts "fh" opt
do
	case $opt in
		f)
			FORCE_INSTALL=TRUE
			;;
		h)
			help
			exit 0;
			;;
	esac
done

main
