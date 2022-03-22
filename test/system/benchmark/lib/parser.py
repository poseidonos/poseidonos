import argparse
import os


class ArgParser:
    config = ""

    def __init__(self):
        parser = argparse.ArgumentParser(description="benchmark options")
        parser.add_argument(
            "--config", help="set specific json file (default: conf.json)")
        args = parser.parse_args()
        curr_dir = os.path.dirname(os.path.abspath(__file__))
        root_dir = curr_dir[0:len(curr_dir) - 25]
        if args.config is None:
            ArgParser.config = root_dir + "test/system/benchmark/config/vm_fio_precommit.json"
        else:
            ArgParser.config = root_dir + "test/system/benchmark/" + args.config

    @classmethod
    def GetConfig(cls):
        return cls.config
