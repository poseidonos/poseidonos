#!/usr/bin/env python3
import sys
sys.path.append("../lib/")
import cli

def main(event_name, perf_impact):
    out = cli.update_event_qos(event_name, perf_impact)
    print (out)

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])