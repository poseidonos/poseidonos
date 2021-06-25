#/usr/bin/python

import commands


def get_event_dic(filename):
    res = {}  # int -> event name
    event_to_val = {}  # event name -> int
    enum_started = False
    with open(filename, "r") as input_fd:
        prev_enum_val = None
        for line in input_fd.readlines():
            if "enum class" in line:
                enum_started = True
                continue
            elif not enum_started:
                continue
            if "=" in line:
                splited = line.strip().split("=")
                event_name = splited[0].strip()
                event_val_str = splited[1].split(",")[0].strip()
                if event_val_str.isdigit() or event_val_str.startswith("0x"):
                    event_val = int(event_val_str, 0)
                else:
                    # second chance
                    try:
                        if "-" in event_val_str:  # e.g., MBR_COUNT = MBR_END - MBR_START
                            event_val_splited = event_val_str.split(" - ")
                            event_val = event_to_val[event_val_splited[0].strip()] - event_to_val[event_val_splited[1].strip()]
                        else:  # MBR_ALLOCATE_MEMORY = MBR_START
                            event_val = event_to_val[event_val_str]
                    except Exception as e:
                        print "Ignored: ", line, str(e)
                        continue
                res[event_val] = event_name
                event_to_val[event_name] = event_val
                prev_enum_val = event_val
            elif not prev_enum_val:
                continue
            elif line.strip().startswith("//"):
                continue
            else:
                if "," not in line.strip():
                    continue
                event_name = line.strip().split(",")[0].strip()
                event_val = prev_enum_val + 1
                res[event_val] = event_name
                event_to_val[event_name] = event_val
                prev_enum_val = event_val
    return res


def extract_loglevel_and_log(event_name):
    if event_name == "SUCCESS":
        return "-", "-"
    cmd = "grep '%s' ../../src/* -R -A 5 -h | grep -v 'pos_event_id.h' | grep -v '_test.cpp'" % event_name
    output = commands.getoutput(cmd)
    loglevel = None
    if "POS_TRACE_" in output:
        loglevel = output.split("POS_TRACE_")[1].split("(")[0]
    elif "POS_REPORT_" in output:
        loglevel = output.split("POS_REPORT_")[1].split("(")[0]
    else:
        loglevel = "-"
    log_msg = "-"
    event_pos = output.find(event_name)
    quote_start = output.find('"', event_pos)
    if quote_start != -1:
        quote_end = output.find('"', quote_start + 1)
        if quote_end != -1:
            log_msg = output[quote_start + 1:quote_end].replace("\n", "")
    return loglevel, log_msg


def to_markdown_table(event_val_to_name):
    table = "|Event Code|Event Name|Severity|Description(s)|\n"
    table += "|----------|----------|--------|------------|\n"
    key_list = event_val_to_name.keys()
    key_list.sort()
    for key in key_list:
        event_name = event_val_to_name[key]
        if event_name.endswith("_END"):
            continue
        elif event_name.endswith("_COUNT"):
            continue
        log_level, log_msg = extract_loglevel_and_log(event_name)
        print event_name, log_level, log_msg
        table += "| %s | %s | %s | %s |\n" % (key, event_name, log_level, log_msg)
    table += "--------------------------\n"
    return table


def main():
    event_val_to_name = get_event_dic("../../src/include/pos_event_id.h")
    table = to_markdown_table(event_val_to_name)
    print table


if __name__ == "__main__":
    main()
