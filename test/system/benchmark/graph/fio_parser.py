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


def GetEtaData(data, file, title):
    try:  # data는 Fio의 eta_data 딕셔너리
        data[title] = {}  # title이 graph의 subplot 이고 빈 딕셔너리 생성
        data[title]["title"] = title  # subplot 이름 설정
        data[title]["x"] = []  # subplot x 축 빈 리스트 생성
        data[title]["bw_read"] = []  # subplot y 축 (bw_read) 빈 리스트 생성
        data[title]["bw_write"] = []  # subplot y 축 (bw_write) 빈 리스트 생성
        data[title]["iops_read"] = []  # subplot y 축 (iops_read) 빈 리스트 생성
        data[title]["iops_write"] = []  # subplot y 축 (iops_write) 빈 리스트 생성

        fp = open(file, "r")
        lines = fp.readlines()
        for line in lines:
            # Jobs: 3 (f=3): [W(3)][20.8%][r=0KiB/s,w=2027KiB/s][r=0,w=4055 IOPS][eta 00m:19s]
            strings = line.split("[")
            if (6 <= len(strings)):
                percentage = strings[2].split("%")[0]
                if "-" in percentage:
                    continue
                bandwidth = strings[3].split(",")
                bw_read = bandwidth[0].split("=")[1].split("iB/s")[0]
                bw_write = bandwidth[1].split("=")[1].split("iB/s")[0]
                iops = strings[4].split(",")
                iops_read = iops[0].split("=")[1]
                iops_write = iops[1].split("=")[1].split(" ")[0]
                data[title]["x"].append(float(percentage))
                data[title]["bw_read"].append(ToFloat(bw_read))
                data[title]["bw_write"].append(ToFloat(bw_write))
                data[title]["iops_read"].append(ToFloat(iops_read))
                data[title]["iops_write"].append(ToFloat(iops_write))
        fp.close()
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        fp.close()


def GetResultData(data, file, title):
    try:  # data는 Fio의 result_data 리스트
        for i in range(len(data)):
            data[i]["index"].append(title)  # subplot 이름 설정

        with open(file, "r") as f:  # result format이 json 임을 가정
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
    try:  # data는 Fio의 log_data 딕셔너리
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
