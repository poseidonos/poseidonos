import os
import yaml
import argparse
import time

prometheus_default_config_yaml = '''
global:
  scrape_interval: 5s
scrape_configs:
  - job_name: ''
    static_configs:
      - targets: []
'''

parser = argparse.ArgumentParser(description='Prometheus config generator', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('--job', required=True)
parser.add_argument('--target', required=True)
parser.add_argument('--output', required=True)

args = parser.parse_args()

config_yaml = yaml.load(prometheus_default_config_yaml, Loader=yaml.SafeLoader)

config_yaml['scrape_configs'][0]['job_name'] = args.job
config_yaml['scrape_configs'][0]['static_configs'][0]['targets'].append(args.target)

config_yaml_file = args.output
with open(config_yaml_file, 'w') as f:
    yaml.dump(config_yaml, f, default_flow_style=False, sort_keys=False)
