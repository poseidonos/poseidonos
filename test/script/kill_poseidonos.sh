#!/bin/bash

echo "kill poseidonos now..."
pgrep poseidonos | xargs kill -9
echo "poseidonos killed.."

