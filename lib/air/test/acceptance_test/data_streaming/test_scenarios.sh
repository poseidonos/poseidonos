#!/bin/bash

for((i=0;i<5;i++)); do
    ./data_stream runtime:10 output:$(($i))_normal.xml
done
