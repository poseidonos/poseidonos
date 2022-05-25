#  Learning POS Management Tool

##  Install
- All Management S/W

```
$ sudo $POS_MGMT_HOME /m9k/install_all.sh

# 1. installs dependent libraries such as libcurl, influxdb, kapacitor, chronograf, nginx-common, libnginx-mod-http-echo, and nginx-light from local deb packages ("nas") or internet ("apt")
# 2. starts up dependent services such as influxdb, chronograf, and kapacitor
# 3. configures and restarts nginx ``# 4. configures env variables for influxdb configuration such as retention policy, query config, and logging level
```

## Run
POS management tool is built as a set of processes, all of which can be run by a single bash script:

```
$ sudo $POS_MGMT_HOME /m9k/run_all.sh

# 1. cleans up the previous deployment of dagent at /usr/local/dagent and redeploys it
# 2. cleans up the previous deployment of magent at /var/log/m9k, restarts influxdb, and redeploys magent
# 3. restarts influxdb, kapacitor, and start-iBofMtool (i..e, mtool)
```

Once all of the processes start up to run, you could access the GUI tool.

## Uninstall
The following will stop the services and delete their soft-links:

```
# disable and remove mtool
$ sudo systemctl disable start-iBofMtool.service
$ sudo systemctl stop start-iBofMtool.service
$ sudo rm -f /usr/local/m9k

# disable and remove dagent
$ sudo systemctl disable dagent.service
$ sudo systemctl stop dagent
$ sudo rm -f /usr/local/dagent

# disable and remove magent
$ sudo systemctl disable magent.service
$ sudo systemctl stop magent
$ sudo rm -f /usr/local/magent
```

The following will stop the dependent services:

```
$ sudo service kapacitor stop
$ sudo service influxdb stop
```

## Configuration
The management IP and port of nginx can be set as follows:

```
$ sudo vi /etc/nginx/conf.d/virtual.conf
$ sudo service nginx reload
```

Upon RUNIBOFOS http call, m9k executes `~/m9k/dagent/script/run_os.sh` to start up POS. The bash command has a hard-coded path to the application binary, which is` /root/workspace/ibofos` by default. You could replace `root_dir` variable with the actual path if it differs from the default value.  

## Integrating with External Application
POS management tool provides REST APIs to help workflow automation. The whole set of the APIs is documented at [REST API](https://github.com/poseidonos/poseidonos/tree/main/doc/guides/rest_api) with sample inputs/outputs captured at the following:

- ```$POS_MGMT_HOME/m9k/dagent/postman/D-Agent.postman_collection.json```
- ```$POS_MGMT_HOME/m9k/dagent/postman/D-Agent.postman_environment.json```

