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


def FormatMsTime(x, idx=0):
    if x >= 1e3:
        return f"{round(x/1e3)}"
    else:
        return f"{round(x)}"


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
        return f"{round(y/1e9, 1)}GiB/s"
    elif y >= 1e6:
        return f"{round(y/1e6, 1)}MiB/s"
    elif y >= 1e3:
        return f"{round(y/1e3, 1)}KiB/s"
    else:
        return f"{round(y)}B/s"


def FormatKBW(y, idx=0):
    if y >= 1e9:
        return f"{round(y/1e9, 1)}TiB/s"
    elif y >= 1e6:
        return f"{round(y/1e6, 1)}GiB/s"
    elif y >= 1e3:
        return f"{round(y/1e3, 1)}MiB/s"
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
        plt.clf()
        num_graph = len(graph_list)
        fig = plt.figure(figsize=(8, 3 * num_graph))

        for i in range(num_graph):
            type = graph_list[i]
            ax = plt.subplot(num_graph, 1, i + 1)
            ax.set_title(type, fontsize=12)
            ax.grid(True, axis="y", color="lightgrey", zorder=0)
            plt.xlabel("time (sec)", fontsize=9)
            plt.ylim([0, 7000000000])
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
                            label=v["title"])
                plt.plot(v["x"], v[type])
            plt.legend(fontsize=8, loc="upper left", ncol=2)

        plt.tight_layout()
        plt.savefig(f"{pic_name}_eta.png", dpi=200)
        plt.close(fig)
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        plt.close(fig)


def DrawCsv(data, pic_name):
    try:
        plt.clf()
        fig = plt.figure(figsize=(8, 3))

        type = "bandwidth"
        ax = plt.subplot(1, 1, 1)
        ax.set_title(type, fontsize=12)
        ax.grid(True, axis="y", color="lightgrey", zorder=0)
        plt.xlabel("time (sec)", fontsize=9)
        ax.xaxis.set_major_formatter(ticker.FuncFormatter(FormatMsTime))
        ax.yaxis.set_major_formatter(ticker.FuncFormatter(FormatBW))
        ax.tick_params(axis='y', labelrotation=30, labelsize=8)
        for v in data.values():
            plt.plot(v["x"], v[type], label=v["title"])
        plt.legend(fontsize=8, loc="upper left", ncol=2)

        plt.tight_layout()
        plt.savefig(f"{pic_name}_bandwidth.png", dpi=200)
        plt.close(fig)
    except Exception as e:
        lib.printer.red(f"{__name__} [Error] {e}")
        plt.close(fig)


def DrawResult(data, pic_name):
    try:
        plt.clf()
        fig = plt.figure(figsize=(12, 12))
        prop_cycle = plt.rcParams["axes.prop_cycle"]
        color_list = prop_cycle.by_key()["color"]

        for i in range(12):
            ax = plt.subplot(4, 3, i + 1)
            ax.set_title(data[i]["title"], fontsize=12)
            ax.grid(True, axis="x", color="lightgrey", zorder=0)
            hbars = ax.barh(  # draw horizontal bar graph
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
            for rect in rects:
                x_val = rect.get_width()
                y_val = rect.get_y() + rect.get_height() / 2
                label = FormatSimpleFloat(x_val)
                x_offset = 5
                align = "left"
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
        plt.clf()
        num_graph = len(data)
        fig = plt.figure(figsize=(8, 3 * num_graph))

        for job in data:
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
                        ["read"]["y"], s=10, label="read")
            plt.plot(data[job]["read"]["x"], data[job]["read"]["y"])
            plt.scatter(data[job]["write"]["x"], data[job]
                        ["write"]["y"], s=10, label="write")
            plt.plot(data[job]["write"]["x"], data[job]["write"]["y"])
            plt.legend(fontsize=8, loc="upper left", ncol=2)

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
