#!/bin/bash

logfile="ibofos.log"

echo "kill ibofos now..."
pgrep ibofos | xargs kill -9
echo "ibofos killed.."

