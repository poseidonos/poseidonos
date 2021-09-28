import lib
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np


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


def FormatSimpleFloat(y, pos=1):
    if y >= 1e9:
        return f"{round(y/1e9, pos)}"
    elif y >= 1e6:
        return f"{round(y/1e6, pos)}"
    elif y >= 1e3:
        return f"{round(y/1e3, pos)}"
    else:
        return f"{round(y, pos)}"


def DrawEta(data, pic_name, graph_list):
    try:
        plt.clf()  # plot 초기화
        num_graph = len(graph_list)
        fig = plt.figure(figsize=(8, 3 * num_graph))  # plot size 설정(unit: inch)

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
                plt.scatter(v["x"], v[type], s=10, label=v["title"])  # 점 그래프 그리기
                plt.plot(v["x"], v[type])  # 선 그래프 그리기
            plt.legend(fontsize=8, loc="upper left", ncol=2)  # 범례 그리기

        plt.tight_layout()
        plt.savefig(f"{pic_name}_eta.png", dpi=200)
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
                ax.xaxis.set_major_formatter(ticker.FuncFormatter(FormatLatency))
            elif "iops" in data[i]["title"]:
                ax.xaxis.set_major_formatter(ticker.FuncFormatter(FormatIOPS))
            elif "bw" in data[i]["title"]:
                ax.xaxis.set_major_formatter(ticker.FuncFormatter(FormatBW))
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
