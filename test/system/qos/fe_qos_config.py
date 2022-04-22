#!/usr/bin/python

import getopt
import sys
import os
import json

setConfig = "true"
feEnable = "true"
rebuildImpact = "low"
conf_file = "/../../../config/ibofos_for_perf_ci.conf"

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path + "./")


def config_change(feEnable, impact):
    with open(current_path + conf_file, "r") as jsonFile:
        data = json.load(jsonFile)

    with open("default_ibofos.conf", "w") as jsonFile:
        json.dump(data, jsonFile, indent=4)

    if ("fe_qos" not in data):
        data["fe_qos"] = {}
    if ("perf_impact" not in data):
        data["perf_impact"] = []

    if (feEnable == "true"):
        data["fe_qos"]["enable"] = True
    elif (feEnable == "false"):
        data["fe_qos"]["enable"] = False

    if (impact == "high"):
        data["perf_impact"]["rebuild"] = "high"
    elif (impact == "low"):
        data["perf_impact"]["rebuild"] = "low"

    with open("/etc/pos/pos.conf", "w") as jsonFile:
        json.dump(data, jsonFile, indent=4)


def config_reset():
    with open("default_ibofos.conf", "r") as jsonFile:
        data = json.load(jsonFile)

    with open("/etc/conf/pos.conf", "w") as jsonFile:
        json.dump(data, jsonFile, indent=4)


def help():
    print('Use below command:')
    print('config.py -s true/false -v true/false -f true/false -i high/low')


def main(argv):
    global conf_file
    try:
        opts, args = getopt.getopt(argv, "hs:v:f:i:", ["set", "fe=", "impact=", "vm="])
    except getopt.GetoptError:
        help()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            help()
            sys.exit()
        elif opt in ("-v", "--vm"):
            if ("true" == arg):
                conf_file = "/../../../config/ibofos_for_vm_ci.conf"
        elif opt in ("-s", "--set"):
            if ("true" != arg and "false" != arg):
                help()
                sys.exit(1)
            setConfig = arg
        elif opt in ("-f", "--fe"):
            if ("true" != arg and "false" != arg):
                help()
                sys.exit(1)
            feEnable = arg
        elif opt in ("-i", "--impact"):
            if ("high" != arg and "low" != arg):
                help()
                sys.exit(1)
            rebuildImpact = arg

    if ("true" == setConfig):
        config_change(feEnable, rebuildImpact)
    else:
        config_reset()


if __name__ == '__main__':
    main(sys.argv[1:])
