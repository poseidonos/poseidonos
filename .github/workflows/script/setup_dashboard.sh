#!/bin/bash

check_environment_variable()
{
    if [ -z ${POS_WORKING_DIR+x} ]; then echo "POS_WORKING_DIR is unset"; NOT_OK=1; fi

    if [ -z ${GA_COMMIT_HASH+x} ]; then echo "GA_COMMIT_HASH is unset"; NOT_OK=1; fi
    if [ -z ${GA_WORKFLOW+x} ]; then echo "GA_WORKFLOW is unset"; NOT_OK=1; fi

    if [ -z ${GRAFANA_DASHBOARD+x} ]; then echo "GRAFANA_DASHBOARD is unset"; NOT_OK=1; fi

    if [ -z ${ESTIMATED_EXECUTION_TIME+x} ]; then echo "ESTIMATED_EXECUTION_TIME is unset"; NOT_OK=1; fi

    if ! [ -z ${NOT_OK+x} ]; then exit 1; fi
}

make_dashboard_link()
{
	START_TIME=$(( $(date +%s) * 1000 ))

    GRAFANA_DASHBOARD_DIRECT=$(echo ${GRAFANA_DASHBOARD}"&refresh=10s&from=${START_TIME}&to=${START_TIME}+${ESTIMATED_EXECUTION_TIME}&var-ga_commit_hash=${GA_COMMIT_HASH}&var-ga_workflow=${GA_WORKFLOW}" | sed 's/#/%23/g')

    cat > ${POS_WORKING_DIR}/Dashboard.html <<-EOF
		<html>
			<body>
				<p> Redirect to Dashboard soon </p>
				<script>
					setTimeout(() => {
						window.location.href = '${GRAFANA_DASHBOARD_DIRECT}';
					}, 2);
				</script>
			</body>
		</html>
	EOF
}

check_environment_variable
make_dashboard_link
