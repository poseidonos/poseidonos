#!/bin/bash

cd ../../../../
make at_rsc_usage_empty cfg=at_rsc_usage_empty
mv bin/rsc_empty_air ./
make at_rsc_usage_no_air cfg=at_rsc_usage_no_air
mv bin/rsc_no_air ./
make at_rsc_usage_perf cfg=at_rsc_usage_perf
mv bin/rsc_perf_node ./
make at_rsc_usage_lat cfg=at_rsc_usage_lat
mv rsc_* bin/
