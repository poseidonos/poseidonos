import os
import yaml
import sys
import argparse
import subprocess
import uuid
import time
import pytz
import datetime

def validate_process(p):
    if p.returncode == 0:
        print('Completed')
    else:
        print('Error')
        print(p.stderr)
        exit()

filebeat_default_config_yaml = '''
filebeat.inputs:
  - type: log
    enabled: true
    close_eof: true
    paths: []
    tags: []
    fields:
      import_uuid: null
      timezone: null

output.logstash:
  hosts: []
'''

default_tag='pos,imported'

root_dir = os.path.dirname(os.path.abspath(__file__))

# Get argument
parser = argparse.ArgumentParser(description='POS Log Importer', formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('--dir', '-d', required=True, help='Specify a directory where input log file(s) exist')
parser.add_argument('--uuid', required=False, default=uuid.uuid1(), help='Specify an uuid of the set of logs')
parser.add_argument('--tag', required=False, default=default_tag, help='Specify tag(s) separated with comma.')
parser.add_argument('--dst', required=False, default='12.36.192.145:5046', help='Specify a log monitoring system destination')
parser.add_argument('--tz', required=False, default='UTC', help='Specify a timezone of input log')

args = parser.parse_args()

# Build filebeat configuration file (filebeat.yml)
config_yaml = yaml.load(filebeat_default_config_yaml, Loader=yaml.SafeLoader)

config_yaml['filebeat.inputs'][0]['paths'].append(os.path.join(os.path.abspath(args.dir), '*'))

for t in default_tag.split(','):
    config_yaml['filebeat.inputs'][0]['tags'].append(t.strip())

if args.tag != default_tag:
    for t in args.tag.split(','):
        config_yaml['filebeat.inputs'][0]['tags'].append(t.strip())

date = datetime.datetime.now().strftime('%Y.%m.%d')
tm = time.localtime()
seconds = tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec

config_yaml['filebeat.inputs'][0]['fields']['import_time'] = 't'.join([date, str(seconds)])
config_yaml['filebeat.inputs'][0]['fields']['import_uuid'] = str(args.uuid)

if args.timezone in pytz.all_timezones:
    config_yaml['filebeat.inputs'][0]['fields']['timezone'] = args.timezone
else:
    raise ValueError('Invalid timezone -> ' + args.timezone)

config_yaml['output.logstash']['hosts'].append(args.destination)

# Dump configuration for importing
print('\n========== Configuration for importing ==========')
yaml.dump(config_yaml, sys.stdout, default_flow_style=False, sort_keys=False)
print()

# Save configuration file
config_yaml_file = os.path.join(root_dir, 'filebeat.yml')
with open(config_yaml_file, 'w') as f:
    os.chmod(config_yaml_file, 0o644)
    yaml.dump(config_yaml, f, default_flow_style=False, sort_keys=False)

# Start importing procedure
start_time = time.time()

print('\n========== Procedure ===========')

print('  Preparing filebeat...', end='\t', flush=True)
if os.path.isfile(os.path.join(root_dir, 'filebeat')) is not True:
    prepare_filebeat_process = subprocess.run(['tar', '-xzvf', os.path.join(root_dir, '../', 'filebeat-7.0.0-linux-x86_64.tar.gz'), '-C', root_dir], capture_output=True)
    validate_process(prepare_filebeat_process)
else:
    print('Completed')

print('  Cleaning registry...', end='\t', flush=True)
delete_registry_process = subprocess.run(' '.join(['rm', '-rf', os.path.join(root_dir, 'filebeat-7.0.0-linux-x86_64', 'data', '*')]), capture_output=True, shell=True)
validate_process(delete_registry_process)

print('  Starting import...', end='\t', flush=True)
run_filebeat_process = subprocess.run([os.path.join(root_dir, 'filebeat-7.0.0-linux-x86_64', 'filebeat'), '--once', '-v', '--path.config', root_dir], capture_output=True)
validate_process(run_filebeat_process)

print('  Completed!!!')
print('\nExecution time for importing logs: %.2f sec' % (time.time() - start_time))
