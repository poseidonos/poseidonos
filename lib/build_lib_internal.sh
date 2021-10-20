#/bin/bash

ROOT_DIR=$(readlink -f $(dirname $0))/..

${ROOT_DIR}/lib/build_ibof_lib.sh all
