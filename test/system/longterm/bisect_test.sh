#!/bin/bash

# change working directory to where script exists

dirpath=`dirname $0`
cd ${dirpath}
rootdir=$(readlink -f ${dirpath})/../../..
curdir=`pwd`

badcommit=$1
goodcommit=$2

cd ${rootdir};
sudo git bisect start ${badcommit} ${goodcommit}

sudo git bisect run ${curdir}/bisect_check_ibofos_process.sh

sudo git bisect reset

cd -;

