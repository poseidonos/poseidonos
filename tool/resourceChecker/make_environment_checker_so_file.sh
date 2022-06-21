#!/bin/bash
rm -f ./environment_checker.so
rm -f ./environment_checker.so.1.0.0

g++ -c -fPIC -Wall ../../src/resource_checker/environment_checker.cpp -o environment_checker.o
g++ -shared -Wl,-soname,environment_checker.so.1 -o environment_checker.so.1.0.0 environment_checker.o
ln -s environment_checker.so.1.0.0 environment_checker.so