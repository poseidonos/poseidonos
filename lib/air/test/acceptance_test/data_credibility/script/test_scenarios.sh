#!/bin/bash

for((i=0;i<1;i++)); do
    ./../../../../bin/data_crd size:32k wl:sw sleep:1us runtime:60 error_margin:5 confidence_interval:95 output:$(( $(($i * 7)) + 1))_32k_sw_1us_em5_ci95.xml
    ./../../../../bin/data_crd size:32k wl:sr sleep:10us runtime:60 error_margin:5 confidence_interval:95 output:$(( $(($i * 7)) + 2))_32k_sr_10us_em5_ci95.xml
    ./../../../../bin/data_crd size:4k wl:rw sleep:100us runtime:60 error_margin:5 confidence_interval:95 output:$(( $(($i * 7)) + 3))_4k_rw_100us_em5_ci95.xml
    ./../../../../bin/data_crd size:4k wl:rr sleep:1ms runtime:60 error_margin:5 confidence_interval:95 output:$(( $(($i * 7)) + 4))_4k_rr_1ms_em5_ci95.xml
    ./../../../../bin/data_crd size:4k wl:rw30 sleep:10ms runtime:60 error_margin:5 confidence_interval:95 output:$(( $(($i * 7)) + 5))_4k_rw30_10ms_em5_ci95.xml
    ./../../../../bin/data_crd size:4k wl:rw30 sleep:100ms runtime:60 error_margin:5 confidence_interval:95 output:$(( $(($i * 7)) + 6))_4k_rw30_100ms_em5_ci95.xml
    ./../../../../bin/data_crd size:4k wl:rw30 sleep:0 runtime:60 error_margin:5 confidence_interval:0 output:$(( $(($i * 7)) + 7))_4k_rw30_0ns_em5_ci0.xml
done
