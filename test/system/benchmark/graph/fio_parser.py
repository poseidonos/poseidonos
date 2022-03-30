import json
import lib
import os


def ToFloat(str_val):
    if "K" in str_val or "k" in str_val:
        return 1000 * float(str_val[:len(str_val) - 1])
    if "M" in str_val or "m" in str_val:
        return 1000000 * float(str_val[:len(str_val) - 1])
    if "G" in str_val or "g" in str_val:
        return 1000000000 * float(str_val[:len(str_val) - 1])
    return float(str_val)


def GetCsvData(data, file, title):
    try:
        data[title] = {}
        data[title]["title"] = title
        data[title]["x"] = []
        data[title]["bandwidth"] = []

        fp = open(file, "r")
        lines = fp.readlines()
        for line in lines:
            # 100, 232960, 1, 0
            strings = line.split(", ")
            if (4 <= len(strings)):
                data[title]["x"].append(int(strings[0]))
                data[title]["bandwidth"].append(int(strings[1]))
        fp.close()
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        fp.close()


def GetEtaData(data, file, title, use_time_axis=False):
    try:
        data[title] = {}
        data[title]["title"] = title
        data[title]["x"] = []
        data[title]["bw_read"] = []
        data[title]["bw_write"] = []
        data[title]["iops_read"] = []
        data[title]["iops_write"] = []

        fp = open(file, "r")
        lines = fp.readlines()
        second = 0
        for line in lines:
            # line example : Jobs: 3 (f=3): [W(3)][20.8%][r=0KiB/s,w=2027KiB/s][r=0,w=4055 IOPS][eta 00m:19s]
            strings = line.split("[")
            if (3 <= len(strings) and "Jobs:" in strings[0]):
                percentage = strings[2].split("%")[0]
                if "-" in percentage:
                    continue
                if use_time_axis is True:
                    data[title]["x"].append(float(second))
                    second += 1
                else:
                    data[title]["x"].append(float(percentage))
                if(len(strings) is 4):
                    data[title]["bw_read"].append(float(0))
                    data[title]["bw_write"].append(float(0))
                    data[title]["iops_read"].append(float(0))
                    data[title]["iops_write"].append(float(0))
                    continue
                bandwidth = strings[3]

                if "r=" in bandwidth and "w=" in bandwidth:
                    bw_read = bandwidth.split(",")[0].split("=")[1].split("iB/s")[0]
                    data[title]["bw_read"].append(ToFloat(bw_read))
                    bw_write = bandwidth.split(",")[1].split("=")[1].split("iB/s")[0]
                    data[title]["bw_write"].append(ToFloat(bw_write))
                elif ("r=" in bandwidth):
                    bw_read = bandwidth.split("=")[1].split("iB/s")[0]
                    data[title]["bw_read"].append(ToFloat(bw_read))
                elif ("w=" in bandwidth):
                    bw_write = bandwidth.split("=")[1].split("iB/s")[0]
                    data[title]["bw_write"].append(ToFloat(bw_write))

                iops = strings[4]
                if "r=" in iops and "w=" in iops:
                    iops_read = iops.split(",")[0].split("=")[1].split(" ")[0]
                    data[title]["iops_read"].append(ToFloat(iops_read))
                    iops_write = iops.split(",")[1].split("=")[1].split(" ")[0]
                    data[title]["iops_write"].append(ToFloat(iops_write))
                elif ("r=" in iops):
                    iops_read = iops.split("=")[1].split(" ")[0]
                    data[title]["iops_read"].append(ToFloat(iops_read))
                elif ("w=" in iops):
                    iops_write = iops.split("=")[1].split(" ")[0]
                    data[title]["iops_write"].append(ToFloat(iops_write))
        fp.close()
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        fp.close()


def GetResultData(data, file, title):
    try:
        for i in range(len(data)):
            data[i]["index"].append(title)

        with open(file, "r") as f:
            json_data = json.load(f)
            data[0]["value"].append(json_data["jobs"][0]["read"]["iops"])
            data[1]["value"].append(json_data["jobs"][0]["read"]["bw"])
            data[2]["value"].append(
                json_data["jobs"][0]["read"]["clat_ns"]["mean"])
            data[3]["value"].append(
                json_data["jobs"][0]["read"]["clat_ns"]["percentile"]["99.900000"])
            data[4]["value"].append(
                json_data["jobs"][0]["read"]["clat_ns"]["percentile"]["99.990000"])
            data[5]["value"].append(
                json_data["jobs"][0]["read"]["clat_ns"]["max"])
            data[6]["value"].append(json_data["jobs"][0]["write"]["iops"])
            data[7]["value"].append(json_data["jobs"][0]["write"]["bw"])
            data[8]["value"].append(
                json_data["jobs"][0]["write"]["clat_ns"]["mean"])
            data[9]["value"].append(
                json_data["jobs"][0]["write"]["clat_ns"]["percentile"]["99.900000"])
            data[10]["value"].append(
                json_data["jobs"][0]["write"]["clat_ns"]["percentile"]["99.990000"])
            data[11]["value"].append(
                json_data["jobs"][0]["write"]["clat_ns"]["max"])
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")


def GetSingleLogFile(data, file, index):
    try:
        data[index] = {}
        data[index]["read"] = {}
        data[index]["read"]["x"] = []
        data[index]["read"]["y"] = []
        data[index]["write"] = {}
        data[index]["write"]["x"] = []
        data[index]["write"]["y"] = []
        fp = open(file, "r")
        lines = fp.readlines()
        for line in lines:
            line_spliter = line.split(',')
            if (4 <= len(line_spliter)):
                if ("0" in line_spliter[2]):
                    data[index]["read"]["x"].append(int(line_spliter[0]))
                    data[index]["read"]["y"].append(int(line_spliter[1]))
                elif ("1" in line_spliter[2]):
                    data[index]["write"]["x"].append(int(line_spliter[0]))
                    data[index]["write"]["y"].append(int(line_spliter[1]))
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")


def GetLogData(data, dir, filename):
    try:
        bw_file_prefix = f"{filename}_bw"
        iops_file_prefix = f"{filename}_iops"
        clat_file_prefix = f"{filename}_clat"

        files = os.listdir(dir)
        files.sort()
        for file in files:
            if bw_file_prefix in file:
                bw_spliter = file.split('.')
                GetSingleLogFile(data["bw"], f"{dir}/{file}", bw_spliter[1])
            elif iops_file_prefix in file:
                iops_spliter = file.split('.')
                GetSingleLogFile(
                    data["iops"], f"{dir}/{file}", iops_spliter[1])
            elif clat_file_prefix in file:
                clat_spliter = file.split('.')
                GetSingleLogFile(
                    data["clat"], f"{dir}/{file}", clat_spliter[1])
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
