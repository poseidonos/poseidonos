#!/usr/bin/env python3
#reusing file from spdk/script/getconfig.py

import os
import re
import sys

comment = re.compile('^\s*#')
assign = re.compile('^\s*(export)\s*([a-zA-Z_]+)\s*(\?)?=\s*([^#]*)')

args = os.environ.copy()
for arg in sys.argv:
    m = assign.match(arg)
    if m:
        var = m.group(1).strip()
        val = m.group(3).strip()
        args[var] = val

defs = {}
try:
    with open("mk/ibof_config.mk") as f:
        for line in f:
            line = line.strip()
            if not comment.match(line):
                m = assign.match(line)
                if m:
                    var = m.group(2).strip()
                    default = m.group(4).strip()
                    val = default
                    if var in args:
                        val = args[var]
                    if default.lower() == 'y' or default.lower() == 'n':
                        if val.lower() == 'y':
                            defs["IBOF_{0}".format(var)] = 1
                        else:
                            defs["IBOF_{0}".format(var)] = 0
                    else:
                        strval = val.replace('"', '\"')
                        defs["IBOF_{0}".format(var)] = strval
except IOError:
    print("mk/ibof_config.mk not found")

for key, value in sorted(defs.items()):
    if value == 0:
        print("#undef {0}".format(key))
    else:
        print("#define {0} {1}".format(key, value))
