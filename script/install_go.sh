#!/bin/bash

FORCE_INSTALL=FALSE
INSTALLED=FALSE
INTERNAL=FALSE

GO_VERSION="go1.18.3"

ARCHIVE_FILE=${GO_VERSION}.linux-amd64.tar.gz
INSTALL_PATH=/usr/local

DEFAULT_URL="https://golang.org/dl/${ARCHIVE_FILE}"
INTERNAL_URL="https://10.227.30.174:7990/projects/IBOF/repos/go/raw/${ARCHIVE_FILE}?at=refs%2Fheads%2Farchive"

DOWNLOAD_URL=${DEFAULT_URL}

check_installation()
{
	if [ -e "${INSTALL_PATH}/go/bin/go" ]
	then
		install_version=`${INSTALL_PATH}/go/bin/go version | awk '{print $3}'`
		if [ $install_version == $GO_VERSION ]
		then
			INSTALLED=TRUE
		fi
	fi
}

download()
{
	if [ ${INTERNAL} == FALSE ]
	then
		wget ${DEFAULT_URL} --timeout 5
		if [ $? != 0 ]
		then
			INTERNAL=TRUE
		fi
	fi

	if [ ${INTERNAL} == TRUE ]
	then
		wget ${INTERNAL_URL} --no-check-certificate -O ${ARCHIVE_FILE}
	fi
}

install()
{
	rm -rf ${INSTALL_PATH}/go
	tar -C ${INSTALL_PATH} -xzf ${ARCHIVE_FILE}
	rm -rf ${ARCHIVE_FILE}
}

help()
{
	echo "Usage: ./install_go.sh [OPTION]"
	echo " -f	force install"
	echo " -i	samsung internal install option"
}


main()
{
	if [ ${FORCE_INSTALL} == FALSE ]
	then
		check_installation
		if [ ${INSTALLED} == TRUE ]
		then
			echo "${GO_VERSION} is already installed."
			exit 0
		fi
	fi

	download

	install
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
			INTERNAL=TRUE
			;;
	esac
done

main
