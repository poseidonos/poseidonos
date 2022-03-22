import lib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import time


def FormatLatency(y, idx=0):
    if y >= 1e9:
        return f"{round(y/1e9)}s"
    elif y >= 1e6:
        return f"{round(y/1e6)}ms"
    elif y >= 1e3:
        return f"{round(y/1e3)}us"
    else:
        return f"{round(y)}ns"


def FormatIOPS(y, idx=0):
    if y >= 1e9:
        return f"{round(y/1e9)}Giops"
    elif y >= 1e6:
        return f"{round(y/1e6)}Miops"
    elif y >= 1e3:
        return f"{round(y/1e3)}Kiops"
    else:
        return f"{round(y)}iops"


def FormatBW(y, idx=0):
    if y >= 1e9:
        return f"{round(y/1e9)}GiB/s"
    elif y >= 1e6:
        return f"{round(y/1e6)}MiB/s"
    elif y >= 1e3:
        return f"{round(y/1e3)}KiB/s"
    else:
        return f"{round(y)}B/s"


def FormatKBW(y, idx=0):
    if y >= 1e9:
        return f"{round(y/1e9)}TiB/s"
    elif y >= 1e6:
        return f"{round(y/1e6)}GiB/s"
    elif y >= 1e3:
        return f"{round(y/1e3)}MiB/s"
    else:
        return f"{round(y)}KiB/s"


def FormatSimpleFloat(y, pos=1):
    if y >= 1e9:
        return f"{round(y/1e9, pos)}"
    elif y >= 1e6:
        return f"{round(y/1e6, pos)}"
    elif y >= 1e3:
        return f"{round(y/1e3, pos)}"
    else:
        return f"{round(y, pos)}"


def FormatEpochTime(y, idx=0):
    time_format = time.strftime("%H:%M:%S", time.localtime(y / 1000))
    return time_format


def DrawEta(data, pic_name, graph_list):
    try:
        plt.clf()  # plot 초기화
        num_graph = len(graph_list)
        # plot size 설정(unit: inch)
        fig = plt.figure(figsize=(8, 3 * num_graph))

        for i in range(num_graph):
            type = graph_list[i]
            ax = plt.subplot(num_graph, 1, i + 1)  # subplot 생성(행, 렬, 순서)
            ax.set_title(type, fontsize=12)
            ax.grid(True, axis="y", color="lightgrey", zorder=0)
            plt.xlabel("percentage", fontsize=9)
            if "iops" in type:
                ax.yaxis.set_major_formatter(ticker.FuncFormatter(FormatIOPS))
            elif "bw" in type:
                ax.yaxis.set_major_formatter(ticker.FuncFormatter(FormatBW))
            else:
                plt.ticklabel_format(axis="y", style="plain")
                ax.yaxis.set_major_formatter(ticker.EngFormatter())
            ax.tick_params(axis='y', labelrotation=30, labelsize=8)
            for v in data.values():
                plt.scatter(v["x"], v[type], s=10,
                            label=v["title"])  # 점 그래프 그리기
                plt.plot(v["x"], v[type])  # 선 그래프 그리기
            plt.legend(fontsize=8, loc="upper left", ncol=2)  # 범례 그리기

        plt.tight_layout()
        plt.savefig(f"{pic_name}_eta.png", dpi=200)
        plt.close(fig)
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        plt.close(fig)


def DrawResultDict(dict_data, pic_name, max_y, x_axis_label, y_axis_label):
    fig = plt.figure(figsize=(12, 12))
    try:
        plt.clf()  # plot 초기화
        plt.rcParams.update({'font.size': 22})
        x_axis = []
        plt.ylim(ymin=0.0, ymax=max_y)
        plt.xlabel(x_axis_label)
        plt.ylabel(y_axis_label)
        for key in dict_data:
            x_axis = []
            for index in range(0, len(dict_data[key])):
                x_axis.append(index)
            plt.plot(x_axis, dict_data[key], label=key)

        plt.legend()
        plt.savefig(f"output/{pic_name}_result.png", dpi=200)
        plt.close(fig)
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        plt.close(fig)


def DrawResult(data, pic_name):
    try:
        plt.clf()  # plot 초기화
        fig = plt.figure(figsize=(12, 12))  # plot size 설정(unit: inch)
        prop_cycle = plt.rcParams["axes.prop_cycle"]
        color_list = prop_cycle.by_key()["color"]

        for i in range(12):
            ax = plt.subplot(4, 3, i + 1)  # subplot 생성(행, 렬, 순서)
            ax.set_title(data[i]["title"], fontsize=12)
            ax.grid(True, axis="x", color="lightgrey", zorder=0)
            hbars = ax.barh(  # 가로 막대 그래프 그리기
                range(len(data[i]["value"])),
                data[i]["value"],
                align="center",
                color=color_list,
                zorder=3
            )
            ax.set_yticks(range(len(data[i]["value"])))
            ax.set_yticklabels(data[i]["index"], fontsize=8)
            ax.invert_yaxis()
            if "lat" in data[i]["title"]:
                ax.xaxis.set_major_formatter(
                    ticker.FuncFormatter(FormatLatency))
            elif "iops" in data[i]["title"]:
                ax.xaxis.set_major_formatter(ticker.FuncFormatter(FormatIOPS))
            elif "bw" in data[i]["title"]:
                ax.xaxis.set_major_formatter(ticker.FuncFormatter(FormatKBW))
            else:
                ax.xaxis.set_major_formatter(ticker.EngFormatter())
            ax.tick_params(axis="x", labelrotation=30, labelsize=8)

            rects = ax.patches
            x_min, x_max = plt.gca().get_xlim()
            for rect in rects:  # 막대에 label 붙여서 값 표시
                x_val = rect.get_width()
                y_val = rect.get_y() + rect.get_height() / 2
                label = FormatSimpleFloat(x_val)
                x_offset = 5
                align = "left"
                # 막대의 크기가 subplot의 3/4보다 크면 label이 subplot을 넘어가는 것 방지
                if 0.75 < (x_val / x_max):
                    x_offset = -10
                    align = "right"
                plt.annotate(
                    label,
                    (x_val, y_val),
                    xytext=(x_offset, 0),
                    textcoords="offset points",
                    va="center",
                    ha=align,
                    fontsize=9
                )

        plt.tight_layout()
        plt.savefig(f"{pic_name}_result.png", dpi=200)
        plt.close(fig)
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        plt.close(fig)


def DrawLogGraphWithType(data, pic_name, type):
    try:
        plt.clf()  # plot 초기화
        num_graph = len(data)
        # plot size setting(unit: inch)
        fig = plt.figure(figsize=(8, 3 * num_graph))

        for job in data:
            # subplot position(행, 렬, 순서)
            ax = plt.subplot(num_graph, 1, int(job))
            ax.set_title(job, fontsize=12)
            ax.grid(True, axis="y", color="lightgrey", zorder=0)
            if ("iops" == type):
                ax.yaxis.set_major_formatter(ticker.FuncFormatter(FormatIOPS))
            elif ("bw" == type):
                ax.yaxis.set_major_formatter(ticker.FuncFormatter(FormatKBW))
            elif ("lat" == type):
                ax.yaxis.set_major_formatter(
                    ticker.FuncFormatter(FormatLatency))
            else:
                ax.yaxis.set_major_formatter(ticker.EngFormatter())
            ax.xaxis.set_major_formatter(ticker.FuncFormatter(FormatEpochTime))
            ax.tick_params(axis='y', labelrotation=30, labelsize=8)
            ax.tick_params(axis='x', labelrotation=0, labelsize=8)
            plt.scatter(data[job]["read"]["x"], data[job]
                        ["read"]["y"], s=10, label="read")  # 점 그래프
            plt.plot(data[job]["read"]["x"], data[job]["read"]["y"])  # 선 그래프
            plt.scatter(data[job]["write"]["x"], data[job]
                        ["write"]["y"], s=10, label="write")
            plt.plot(data[job]["write"]["x"], data[job]["write"]["y"])
            plt.legend(fontsize=8, loc="upper left", ncol=2)  # 범례 그리기

        plt.tight_layout()
        plt.savefig(pic_name, dpi=200)
        plt.close(fig)
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        plt.close(fig)


def DrawLog(data, pic_name):
    DrawLogGraphWithType(data["iops"], f"{pic_name}_per_job_iops.png", "iops")
    DrawLogGraphWithType(data["bw"], f"{pic_name}_per_job_bw.png", "bw")
    DrawLogGraphWithType(data["clat"], f"{pic_name}_per_job_clat.png", "lat")
