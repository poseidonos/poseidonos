#!/bin/bash

while getopts 'k' 'r' opt;
do
    case ${OPTKEY} in
        'k') OPT_KILL='true'
            ;;
        'r') OPT_REMOVE='true'
            ;;
        *)
            echo "UNIMPLEMENTED OPTION -- ${OPTKEY}" >&2
            exit 1
            ;;
    esac
done

[ -z ${OPT_KILL} ] && echo 'Kill filebeat process' && pkill -9 filebeat
[ -z ${OPT_REMOVE} ] && echo 'Clean up log files' && rm -rf /var/log/pos/*