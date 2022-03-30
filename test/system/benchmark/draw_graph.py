#!/usr/bin/env python3

import scenario
import lib
import graph
import sys

eta_raw_data = "211216_143241_randwrite_4k.eta"
if(len(sys.argv) == 2):
    eta_raw_data = sys.argv[1]
csv_raw_data = "211116_193019_fio_4k_randwrite_Initiator02_bw.log"
output_name = eta_raw_data.split(".")[0]

graph_fio = graph.manager.Fio(output_name)
# CSV, bw
# graph_fio.AddCsvData(csv_raw_data, "test")
# graph_fio.DrawCsv()

# ETA
graph_fio.AddEtaData(eta_raw_data, "Initiator02", True)
graph_fio.DrawEta(["bw_write"])
