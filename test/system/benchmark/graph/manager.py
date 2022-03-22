import graph.draw
import graph.fio_parser


class Fio:
    def __init__(self, pic_name):
        self.pic_name = pic_name
        print(self.pic_name)
        self.eta_data = {}
        self.result_data = []
        self.result_data.append(
            {"title": "read_iops", "index": [], "value": []})
        self.result_data.append({"title": "read_bw", "index": [], "value": []})
        self.result_data.append(
            {"title": "read_clat_avg", "index": [], "value": []})
        self.result_data.append(
            {"title": "read_clat_99.9th", "index": [], "value": []})
        self.result_data.append(
            {"title": "read_clat_99.99th", "index": [], "value": []})
        self.result_data.append(
            {"title": "read_clat_max", "index": [], "value": []})
        self.result_data.append(
            {"title": "write_iops", "index": [], "value": []})
        self.result_data.append(
            {"title": "write_bw", "index": [], "value": []})
        self.result_data.append(
            {"title": "write_clat_avg", "index": [], "value": []})
        self.result_data.append(
            {"title": "write_clat_99.9th", "index": [], "value": []})
        self.result_data.append(
            {"title": "write_clat_99.99th", "index": [], "value": []})
        self.result_data.append(
            {"title": "write_clat_max", "index": [], "value": []})
        self.log_data = {}
        self.log_data["bw"] = {}
        self.log_data["iops"] = {}
        self.log_data["clat"] = {}

    def AddEtaData(self, file, title):
        graph.fio_parser.GetEtaData(self.eta_data, file, title)

    def DrawEta(self, graph_list):
        graph.draw.DrawEta(self.eta_data, self.pic_name, graph_list)

    def AddResultData(self, file, title):
        graph.fio_parser.GetResultData(self.result_data, file, title)

    def DrawResult(self):
        graph.draw.DrawResult(self.result_data, self.pic_name)

    def AddLogData(self, dir, filename):
        graph.fio_parser.GetLogData(self.log_data, dir, filename)

    def DrawLog(self, filepath_name):
        graph.draw.DrawLog(self.log_data, filepath_name)

    def ClearLogData(self):
        for data_type in self.log_data:
            for job in self.log_data[data_type]:
                self.log_data[data_type][job]["read"]["x"].clear()
                self.log_data[data_type][job]["read"]["y"].clear()
                self.log_data[data_type][job]["write"]["x"].clear()
                self.log_data[data_type][job]["write"]["y"].clear()
                self.log_data[data_type][job]["read"].clear()
                self.log_data[data_type][job]["write"].clear()
                self.log_data[data_type][job].clear()
            self.log_data[data_type].clear()
        self.log_data.clear()

        self.log_data = {}
        self.log_data["bw"] = {}
        self.log_data["iops"] = {}
        self.log_data["clat"] = {}
