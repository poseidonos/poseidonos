import os
import yaml
import argparse
import time

filebeat_default_config_yaml = '''
filebeat.inputs:
  - type: log
    enabled: true
    paths: []
    tags: []
    fields:
      timezone: null
output.logstash:
  hosts: []
'''

parser = argparse.ArgumentParser(description='Filebeat config generator', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('--log_path', required=True)
parser.add_argument('--tag', required=True)
parser.add_argument('--timezone', required=True)
parser.add_argument('--destination', required=True)
parser.add_argument('--field', required=True)
parser.add_argument('--output', required=True)

args = parser.parse_args()

config_yaml = yaml.load(filebeat_default_config_yaml, Loader=yaml.SafeLoader)

config_yaml['filebeat.inputs'][0]['paths'].append(os.path.join(os.path.abspath(args.log_path), 'pos.log'))

for t in args.tag.split(','):
    config_yaml['filebeat.inputs'][0]['tags'].append(t.strip())

config_yaml['filebeat.inputs'][0]['fields']['timezone'] = args.timezone

for field in args.field.strip().split(','):
    f = field.split(':')
    config_yaml['filebeat.inputs'][0]['fields'][f[0]] = f[1]


config_yaml['output.logstash']['hosts'].append(args.destination)

config_yaml_file = args.output
with open(config_yaml_file, 'w') as f:
    yaml.dump(config_yaml, f, default_flow_style=False, sort_keys=False)
