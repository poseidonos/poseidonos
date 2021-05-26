#!/bin/bash

SCRIPT_PATH=$(dirname $(realpath $0))

if [[ $1 == "localhost" ]] || [[ $1 == "127.0.0.1" ]]
then
  echo "localrun"
  $SCRIPT_PATH/run_os.sh
else
  echo "remoterun"
  sshpass -p bamboo ssh -o StrictHostKeyChecking=no root@$1 "bash -s" < $SCRIPT_PATH/run_os.sh
fi

exit 0
