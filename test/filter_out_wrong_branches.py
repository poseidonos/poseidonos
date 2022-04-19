#!/usr/bin/env python3

import sys
import re


def filter(input_file):
    input_fd = open(input_file, "r")
    last_content = None
    output_fd = open("coverage_filtered_pos.lcov", "w")
    count_BRF = 0  # branches total
    count_BRH = 0  # branches hit
    for line in input_fd.readlines():
        if line.startswith("SF:"):
            last_file = line.split("SF:")[1].strip()
            print("reading ", last_file)
            last_content = open(last_file, "r").read()
            last_content_split = last_content.split("\n")
            branch_intervals = get_if_else_while_expr_sorted_intervals(last_content)
            output_fd.write(line)
            continue
        if line.startswith("BRF:"):
            output_fd.write("BRF:%d\n" % count_BRF)
            count_BRF = 0
            continue
        if line.startswith("BRH:"):
            output_fd.write("BRH:%d\n" % count_BRH)
            count_BRH = 0
            continue
        if not line.startswith("BRDA:"):
            output_fd.write(line)
            continue
        splited = line.strip().split("BRDA:")[1].split(",")
        if len(splited) != 4:
            print("Format isn't expected: ", line)
            continue
        line_num, block, branch_num, branch_count = splited
        src_line_idx = int(line_num) - 1
        src_line = last_content_split[src_line_idx]
        if should_filter_out(src_line, src_line_idx, branch_intervals):
            # let's skip writing this out!
            # print src_line
            continue
        if branch_count == '-':
            pass
        elif int(branch_count) > 0:
            count_BRH += 1
        count_BRF += 1
        output_fd.write(line)
    print("\nNew coverage data has been written to", output_fd.name)
    output_fd.close()


def should_filter_out(src_line, pos_src_line, branch_intervals):
    # filter out wrong branches not in if-else-while expr
    for pos_start, pos_end in branch_intervals:
        if pos_src_line < pos_start:
            return True
        elif pos_src_line >= pos_start and pos_src_line <= pos_end:
            return False
        else:
            continue
    return True


def get_if_else_while_expr_sorted_intervals(src_content):
    intervals = []
    if_pattern = re.compile(r"\sif\s*?[\s\S]+?{")
    else_pattern = re.compile(r"\selse\s*?[\s\S]+?{")
    while_pattern = re.compile(r"\swhile\s*?[\s\S]+?{")

    for pat in [if_pattern, else_pattern, while_pattern]:
        for matched in pat.finditer(src_content):
            pos_start, pos_end = matched.span()
            branch_line_start = src_content[:pos_start].count("\n")
            branch_line_end = branch_line_start + src_content[pos_start:pos_end].count("\n")
            intervals.append((branch_line_start, branch_line_end))
    intervals.sort(key=lambda x: x[0])
    print("[(branch_start, branch_end)] = ", intervals)
    return intervals


def main():
    if len(sys.argv) != 2:
        print("Usage  ) ./filter_out_wrong_branches.py <coverage_file>")
        print("Example) ./filter_out_wrong_branches.py covered_filtered.lcov")
        sys.exit(1)

    input_file = sys.argv[1]
    filter(input_file)


if __name__ == '__main__':
    main()
