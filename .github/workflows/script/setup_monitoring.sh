#!/bin/bash

PROMETHEUS_YAML_PATH=${POS_WORKING_DIR}/config/prometheus.yml

check_environment_variable()
{
    if [ -z ${POS_WORKING_DIR+x} ]; then echo "POS_WORKING_DIR is unset"; NOT_OK=1; fi

    if [ -z ${ID+x} ]; then echo "ID is unset"; NOT_OK=1; fi
    if [ -z ${POS_EXPORTER_PORT+x} ]; then echo "POS_EXPORTER_PORT is unset"; NOT_OK=1; fi
    if [ -z ${AMP_REMOTE_WRITE_URL+x} ]; then echo "AMP_REMOTE_WRITE_URL is unset"; NOT_OK=1; fi
    if [ -z ${AMP_SIGV4_REGION+x} ]; then echo "AMP_SIGV4_REGION is unset"; NOT_OK=1; fi
    if [ -z ${AMP_SIGV4_ACCESS_KEY+x} ]; then echo "AMP_SIGV4_ACCESS_KEY is unset"; NOT_OK=1; fi
    if [ -z ${AMP_SIGV4_SECRET_KEY+x} ]; then echo "AMP_SIGV4_SECRET_KEY is unset"; NOT_OK=1; fi

    if ! [ -z ${NOT_OK+x} ]; then exit 1; fi
}

setup_pos_exporter()
{
    # Clean up prometheus container
    pkill -9 pos-exporter

    # run pos-exporter
    nohup ${POS_WORKING_DIR}/bin/pos-exporter \
            < /dev/null \
            1> ${POS_WORKING_DIR}/bin/pos-exporter_stdout.txt \
            2> ${POS_WORKING_DIR}/bin/pos-exporter_stderr.txt \
            &
}

setup_prometheus()
{
    # Clean up prometheus container
    docker ps -a | grep prometheus | awk '{print $1}' | xargs docker kill;
    docker ps -a | grep prometheus | awk '{print $1}' | xargs docker rm;

    # prometheus config setting
    cat >${PROMETHEUS_YAML_PATH} <<-EOF
			scrape_configs:
			  - job_name: ${ID}
			    scrape_interval: 1s
			    static_configs:
			      - targets: ["host.docker.internal:${POS_EXPORTER_PORT}"]
			remote_write:
			  - url: ${AMP_REMOTE_WRITE_URL}
			    sigv4:
			      region: ${AMP_SIGV4_REGION}
			      access_key: ${AMP_SIGV4_ACCESS_KEY}
			    secret_key: ${AMP_SIGV4_SECRET_KEY}
EOF

    # run prometheus
    docker run \
            -p 9090:9090 \
            --add-host host.docker.internal:host-gateway \
            -v ${PROMETHEUS_YAML_PATH}:/etc/prometheus/prometheus.yml \
            prom/prometheus
}

check_environment_variable

setup_pos_exporter
setup_prometheus