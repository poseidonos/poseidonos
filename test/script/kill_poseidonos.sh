#!/bin/bash

logfile="pos.log"

echo "kill poseidonos now..."
pgrep poseidonos | xargs kill -9
echo "poseidonos killed.."

