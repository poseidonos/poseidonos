#!/usr/bin/python

import getopt
import sys
import json

setConfig = "true"
feEnable = "true"
rebuildImpact = "low"


def config_change(feEnable, impact):
    with open("/etc/pos/pos.conf", "r") as jsonFile:
        data = json.load(jsonFile)

    with open("default_ibofos.conf", "w") as jsonFile:
        json.dump(data, jsonFile, indent=4)

    if ("qos" not in data):
        data["qos"] = {}

    if (feEnable == "true"):
        data["qos"]["fe_enable"] = True
    elif (feEnable == "false"):
        data["qos"]["fe_enable"] = False

    if (impact == "high"):
        data["qos"]["rebuild_impact"] = "high"
    elif (impact == "low"):
        data["qos"]["rebuild_impact"] = "low"

    with open("/etc/pos/pos.conf", "w") as jsonFile:
        json.dump(data, jsonFile, indent=4)


def config_reset():
    with open("default_ibofos.conf", "r") as jsonFile:
        data = json.load(jsonFile)

    with open("/etc/conf/pos.conf", "w") as jsonFile:
        json.dump(data, jsonFile, indent=4)


def help():
    print('Use below command:')
    print('config.py -s true/false -f true/false -i high/low')


def main(argv):
    if (6 != len(argv)):
        help()
        sys.exit(2)
    try:
        opts, args = getopt.getopt(argv, "hs:f:i:", ["set", "fe=", "impact="])
    except getopt.GetoptError:
        help()
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            help()
            sys.exit()
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
