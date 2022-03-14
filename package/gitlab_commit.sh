#!/bin/bash

COMMIT_MSG=$(sed -n -e '/Version:/p' /home/psd/ibofos/package/src/DEBIAN/control | cut -d ' ' -f2)

git commit -m "${COMMIT_MSG}"
