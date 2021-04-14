import sys
sys.path.append("test/system/lib/")
import json_parser
import json
def flush_gcov(result):
    if json_parser.get_response_code(result) != 0:
        print("* Gcov flush failed, check WBT option")
    elif json_parser.get_data_code(result) != 0:
        print("* Gcov flush failed, check gcov option")
    else:
        print("* Flush gcov completed")

if __name__ == '__main__':
    flush_gcov(sys.argv[1])
