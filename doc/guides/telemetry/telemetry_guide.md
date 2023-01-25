This guide introduces how to monitor the metrics of PoseidonOS using POS telemetry, Prometheus, and Grafana. 

## Start PoseidonOS Telemetry
Start PoseidonOS telemetry using the following command. Once the telemetry starts, PoseidonOS will produce the metrics and sends them to pos-exporter.
```
$ sudo {root_dir}/bin/poseidonos-cli telemetry start
```

## Start An Exporter
Execute pos-exporter (note: we recommend running it in background). The default location of pos-exporter is "$YOUR_POS_PATH/bin/".
```
$ sudo $YOUR_POS_PATH/bin/pos-exporter &
```

## Check the Port
- After the above steps, 2112 port may be opened. Make sure that the 2112 port is used by pos-exporter.
```
$ sudo lsof -i:2112 
COMMAND    PID USER   FD   TYPE   DEVICE SIZE/OFF NODE NAME
pos-expor 1011  psd    3u  IPv6 10261122      0t0  TCP *:2112 (LISTEN)
```

## Check the Metrics
If the pos-exporter runs normally, you can check the metrics using the HTTP request.
```
$ curl localhost:2112/metrics
<metrics will be listed....>
```

## Set up Prometheus
For the metrics to be scraped periodically and managed, you may need Prometheus. Install Prometheus using apt. 
```
$ sudo apt install Prometheus
```
The following are the recommended setting for the Prometheus for PoseidonOS. 
```
# Modify the Prometheus config(/etc/Prometheus/Prometheus.yml) as below and restart the Prometheus instance if any.
# Replace "localhost" with an IP address where pos-exporter is running
scrape_configs:
  - job_name: 'poseidonos'
    scrape_interval: 1s
    static_configs:
      - targets: ['localhost:2112']
```
```
# Restarting Prometheus
sudo systemctl restart Prometheus
```
By default, Prometheus provides a web interface using 9090 port. Connect to http://{ip where Prometheus is running}:9090 and explore the metrics using promql (https://Prometheus.io/docs/Prometheus/latest/querying/basics/)