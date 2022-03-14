#!/bin/bash

POS_TARGET_IP=""        # -i
POS_TARGET_USER=""      # -u
POS_TARGET_PW=""        # -p

texecc()
{
    echo "[${POS_TARGET_USER}@${POS_TARGET_IP}]" $@;
    sshpass -p ${POS_TARGET_PW} ssh ${POS_TARGET_USER}@${POS_TARGET_IP} "$@"
}

check_argument()
{
    [ -z ${POS_TARGET_IP} ] && echo "Specify POS_TARGET_IP with -i" && exit 1
    [ -z ${POS_TARGET_USER} ] && echo "Specify POS_TARGET_USER with -u" && exit 1
    [ -z ${POS_TARGET_PW} ] && echo "Specify POS_TARGET_PW with -p" && exit 1
}

while getopts 'kri:u:p:' OPT;
do
    case ${OPT} in
        i) POS_TARGET_IP="$OPTARG"
            ;;
        u) POS_TARGET_USER="$OPTARG"
            ;;
        p) POS_TARGET_PW="$OPTARG"
            ;;
        k) OPT_KILL='true'
            ;;
        r) OPT_REMOVE='true'
            ;;
        *)
            echo "Unimplemented option -- ${OPT}" >&2
            exit 1
            ;;
    esac
done

check_argument

if [ ! -z ${OPT_KILL+x} ]; then
    echo 'Kill filebeat process' && texecc "pkill -9 filebeat"; \
    echo 'Kill prometheus process' && texecc "pkill -9 prometheus"; \
    echo 'Kill pos-exporter' && texecc "pkill -9 pos-exporter"
fi

if [ ! -z ${OPT_REMOVE+x} ]; then
    echo 'Clean up log files' && texecc "rm -rf /var/log/pos/*"
fi