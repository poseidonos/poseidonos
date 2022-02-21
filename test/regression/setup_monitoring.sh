#!/bin/bash

POS_TARGET_IP=""        # -i
POS_TARGET_USER=""      # -u
POS_TARGET_PW=""        # -p
POS_TARGET_ROOT_DIR=""  # -r

POS_LOG_DESTINATION_ADDR=""     # -d
POS_LOG_PATH="/var/log/pos/"

BAMBOO_PLAN_NAME=""             # -n
BAMBOO_BUILD_NUMBER=""          # -b

WORKING_DIR="test/regression/"
CONFIG_DIR="config/"
BINARY_DIR="bin/"

texecc()
{
    echo "[${POS_TARGET_USER}@${POS_TARGET_IP}]" $@;
    sshpass -p ${POS_TARGET_PW} ssh ${POS_TARGET_USER}@${POS_TARGET_IP} "cd ${POS_TARGET_ROOT_DIR}/ibofos; $@"
}

check_argument()
{
    [ -z ${POS_TARGET_IP} ] && echo "Specify POS_TARGET_IP with -i" && exit 1
    [ -z ${POS_TARGET_USER} ] && echo "Specify POS_TARGET_USER with -u" && exit 1
    [ -z ${POS_TARGET_PW} ] && echo "Specify POS_TARGET_PW with -p" && exit 1
    [ -z ${POS_TARGET_ROOT_DIR} ] && echo "Specify POS_TARGET_ROOT_DIR with -r" && exit 1

    [ -z ${BAMBOO_PLAN_NAME} ] && echo "Specify BAMBOO_PLAN_NAME with -n" && exit 1
    [ -z ${BAMBOO_BUILD_NUMBER} ] && echo "Specify BAMBOO_BUILD_NUMBER with -b" && exit 1

    [ -z ${POS_LOG_DESTINATION_ADDR} ] && echo "Specify POS_LOG_DESTINATION_ADDR with -d" && exit 1

    echo "BAMBOO_PLAN_NAME: [${BAMBOO_PLAN_NAME}]"
    echo "BAMBOO_BUILD_NUMBER: [${BAMBOO_BUILD_NUMBER}]"
}

init_monitoring()
{
    # Kill filebeat instance if exists (w/ -k)
    # + Remove all pos logs (w/ -r)
    texecc "echo ${POS_TARGET_PW} | sudo -S ${WORKING_DIR}/clean_monitoring.sh -k -r"
}

setup_filebeat()
{
    # Prepare filebeat executable
    texecc "tar -xzvf tool/log_importer/filebeat-executable.tar.gz -C ${BINARY_DIR} > /dev/null"

    # Setup required library
    texecc "python3 -m pip install --ignore-installed -r ${WORKING_DIR}/packages/requirement.txt > /dev/null"
    
    # Create filebeat config
    texecc "echo ${POS_TARGET_PW} | sudo -S python3 ${WORKING_DIR}/gen_filebeat_config.py \
        --log_path ${POS_LOG_PATH} \
        --output config/filebeat.yml \
        --tag CI \
        --timezone `date +"%Z"` \
        --destination ${POS_LOG_DESTINATION_ADDR} \
        --field bamboo_plan:${BAMBOO_PLAN_NAME},bamboo_build_number:${BAMBOO_BUILD_NUMBER} 2>/dev/null"

    # Clean up filebeat registry & Log
    texecc "rm -rf ${BINARY_DIR}/data ${BINARY_DIR}/logs"
    
    # Execute filebeat
    texecc "nohup bin/filebeat --path.config ${CONFIG_DIR} </dev/null 1>>${BINARY_DIR}/filebeat_stdout.txt 2>${BINARY_DIR}/filebeat_stderr.txt &"
}

setup_monitoring()
{
    # For Log
    setup_filebeat

    # TBD (Metrics, etc...)
}

while getopts "i:u:p:r:d:n:b:" opt
do
    case $opt in
        i) POS_TARGET_IP="$OPTARG"
            ;;
        u) POS_TARGET_USER="$OPTARG"
            ;;
        p) POS_TARGET_PW="$OPTARG"
            ;;
        r) POS_TARGET_ROOT_DIR="$OPTARG"
            ;;
        d) POS_LOG_DESTINATION_ADDR="$OPTARG"
            ;;
        n) BAMBOO_PLAN_NAME=$(echo ${OPTARG} | sed -e 's/ /_/g')
            ;;
        b) BAMBOO_BUILD_NUMBER=$(echo ${OPTARG} | sed 's/'\''//g')
    esac
done

check_argument
init_monitoring
setup_monitoring