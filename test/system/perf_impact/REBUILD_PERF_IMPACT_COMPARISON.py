#!/usr/bin/env python3
import os
import sys
import REBUILD_PERF_BASIC_1
# Example Usage
if __name__ == "__main__":  
    REBUILD_PERF_BASIC_1.remove_log()
    ipaddr = sys.argv[1]
    REBUILD_PERF_BASIC_1.execute(ipaddr, 'high')
    REBUILD_PERF_BASIC_1.execute(ipaddr, 'medium')
    REBUILD_PERF_BASIC_1.execute(ipaddr, 'low')
    
