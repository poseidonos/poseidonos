import graph.draw
import graph.fio_parser


class Fio:
    def __init__(self, pic_name):
        self.pic_name = pic_name
        print(self.pic_name)
        self.eta_data = {}
        self.result_data = []
        self.result_data.append({"title": "read_iops", "index": [], "value": []})
        self.result_data.append({"title": "read_bw", "index": [], "value": []})
        self.result_data.append({"title": "read_clat_avg", "index": [], "value": []})
        self.result_data.append({"title": "read_clat_99.9th", "index": [], "value": []})
        self.result_data.append({"title": "read_clat_99.99th", "index": [], "value": []})
        self.result_data.append({"title": "read_clat_max", "index": [], "value": []})
        self.result_data.append({"title": "write_iops", "index": [], "value": []})
        self.result_data.append({"title": "write_bw", "index": [], "value": []})
        self.result_data.append({"title": "write_clat_avg", "index": [], "value": []})
        self.result_data.append({"title": "write_clat_99.9th", "index": [], "value": []})
        self.result_data.append({"title": "write_clat_99.99th", "index": [], "value": []})
        self.result_data.append({"title": "write_clat_max", "index": [], "value": []})

    def AddEtaData(self, file, title):
        graph.fio_parser.GetEtaData(self.eta_data, file, title)

    def DrawEta(self, graph_list):
        graph.draw.DrawEta(self.eta_data, self.pic_name, graph_list)

    def AddResultData(self, file, title):
        graph.fio_parser.GetResultData(self.result_data, file, title)

    def DrawResult(self):
        graph.draw.DrawResult(self.result_data, self.pic_name)
