#!/bin/bash

PROMETHEUS_CONF_PATH=${POS_WORKING_DIR}/config/prometheus.yml
FILEBEAT_CONF_PATH=${POS_WORKING_DIR}/config/filebeat.yml
LOGSTASH_CONF_PATH=${POS_WORKING_DIR}/config/logstash.conf

check_environment_variable()
{
    if [ -z ${POS_WORKING_DIR+x} ]; then echo "POS_WORKING_DIR is unset"; NOT_OK=1; fi

    if [ -z ${GA_COMMIT_HASH+x} ]; then echo "GA_COMMIT_HASH is unset"; NOT_OK=1; fi
    if [ -z ${GA_WORKFLOW+x} ]; then echo "GA_WORKFLOW is unset"; NOT_OK=1; fi
    if [ -z ${GA_DETAIL+x} ]; then echo "GA_DETAIL is unset"; NOT_OK=1; fi

    if [ -z ${POS_EXPORTER_PORT+x} ]; then echo "POS_EXPORTER_PORT is unset"; NOT_OK=1; fi
    if [ -z ${AMP_REMOTE_WRITE_URL+x} ]; then echo "AMP_REMOTE_WRITE_URL is unset"; NOT_OK=1; fi
    if [ -z ${AMP_SIGV4_REGION+x} ]; then echo "AMP_SIGV4_REGION is unset"; NOT_OK=1; fi
    if [ -z ${AMP_SIGV4_ACCESS_KEY+x} ]; then echo "AMP_SIGV4_ACCESS_KEY is unset"; NOT_OK=1; fi
    if [ -z ${AMP_SIGV4_SECRET_KEY+x} ]; then echo "AMP_SIGV4_SECRET_KEY is unset"; NOT_OK=1; fi

    if [ -z ${OS_DOMAIN_ENDPOINT+x} ]; then echo "OS_DOMAIN_ENDPOINT is unset"; NOT_OK=1; fi
    if [ -z ${OS_REGION+x} ]; then echo "OS_REGION is unset"; NOT_OK=1; fi
    if [ -z ${LOGSTASH_PORT+x} ]; then echo "LOGSTASH_PORT is unset"; NOT_OK=1; fi

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
    # prometheus config setting
    cat > ${PROMETHEUS_CONF_PATH} <<-EOF
		scrape_configs:
		  - job_name: "Github-Action"
		    scrape_interval: 1s
		    static_configs:
		      - targets: ["$(hostname -i):${POS_EXPORTER_PORT}"]
		        labels:
		          ga_commit_hash: "${GA_COMMIT_HASH}"
		          ga_workflow: "${GA_WORKFLOW}"
		          ga_detail: "${GA_DETAIL}"
		remote_write:
		  - url: ${AMP_REMOTE_WRITE_URL}
		    sigv4:
		      region: ${AMP_SIGV4_REGION}
		      access_key: ${AMP_SIGV4_ACCESS_KEY}
		      secret_key: ${AMP_SIGV4_SECRET_KEY}
		EOF

    # run prometheus
    docker run \
            -d \
            -p 9090:9090 \
            -v ${PROMETHEUS_CONF_PATH}:/etc/prometheus/prometheus.yml \
            prom/prometheus
}

setup_logstash ()
{
    # logstash config setting
    cat > ${LOGSTASH_CONF_PATH} <<-EOF
		input {
		  beats {
		    port => 5045
		    type => "POS-Log-from-Github-Action"
		  }
		}
		filter {
		  grok{
		    match => {
		      "message" => "\[%{TIMESTAMP_ISO8601:[@metadata][time]}\]\[%{INT:process_id}\]\[%{INT:thread_id}\]\[%{INT:pos_id}\]\[%{INT:event_id}\]\[\s%{GREEDYDATA:level}\s\]%{GREEDYDATA:inner_message}"
		    }
		    add_field => {
		      "ga_commit_hash" => "${GA_COMMIT_HASH}"
		      "ga_workflow" => "${GA_WORKFLOW}"
		      "ga_detail" => "${GA_DETAIL}"
		    }
		  }
		  if "_grokparsefailure" in [tag] {
		    # Parsing fails, log is in "JSON (Structured)" format
		    json { source => "message" }
		  }
		  date{
		    match => [ "[@metadata][time]", "yyyy-MM-dd HH:mm:ss.SSSSSSSSS" ]
		    timezone => "UTC"
 		  }
		}
		output {
		  opensearch {
		    hosts  => ["${OS_DOMAIN_ENDPOINT}:443"]
		    auth_type => {
		      type => 'aws_iam'
		      aws_access_key_id => '${AMP_SIGV4_ACCESS_KEY}'
		      aws_secret_access_key => '${AMP_SIGV4_SECRET_KEY}'
		      region => '${OS_REGION}'
		    }
		    index => "github-action-pos-log-$(echo ${GA_COMMIT_HASH} | tr '[:upper:]' '[:lower:]')"
		  }
		}
		EOF

    # run logstash
    docker run \
            -d \
            -p ${LOGSTASH_PORT}:${LOGSTASH_PORT} \
            -v ${LOGSTASH_CONF_PATH}:/usr/share/logstash/pipeline/logstash.conf \
            opensearchproject/logstash-oss-with-opensearch-output-plugin
}

setup_filebeat ()
{
    # filebeat config setting
    cat > ${FILEBEAT_CONF_PATH} <<-EOF
		filebeat.inputs:
		  - type: log
		    enabled: true
		    paths:
		      - /var/log/pos/pos.log
		output.logstash:
		  hosts: ["$(hostname -i):${LOGSTASH_PORT}"]
		EOF

    # run filebeat
    docker run \
            -d \
            -v ${FILEBEAT_CONF_PATH}:/usr/share/filebeat/filebeat.yml \
            docker.elastic.co/beats/filebeat-oss:8.2.0 -e --strict.perms=false
}

check_environment_variable

# For metric monitoring
setup_pos_exporter
setup_prometheus

# For log monitoring
setup_logstash
setup_filebeat