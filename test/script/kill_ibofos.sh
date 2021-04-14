#!/bin/bash

logfile="pos.log"

echo "kill ibofos now..."
pgrep ibofos | xargs kill -9
echo "ibofos killed.."

