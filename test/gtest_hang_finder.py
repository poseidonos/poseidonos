#!/usr/bin/python
import sys
import commands

if len(sys.argv) != 2:
    print "Usage) ./gtest_hang_finder.py <TestPrefix>"
    print " e.g.) ./gtest_hang_finder.py Read"
    print " e.g.) ./gtest_hang_finder.py ReadSubmission"
    sys.exit(1)

prefix = sys.argv[1]
print "Prefix: ", prefix

for upperCase in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
    test_prefix = prefix + upperCase + "*"
    tmpl = 'build/pos_ut --gtest_filter="%s"' % test_prefix
    print tmpl
    print commands.getoutput(tmpl)
