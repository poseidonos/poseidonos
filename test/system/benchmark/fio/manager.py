import copy
import lib


class Fio:
    def __init__(self):
        self.opt = {}
        self.cmd = ""
        self.jobs = []

    def SyncRun(self):
        try:
            cmd = copy.deepcopy(self.cmd)
            subcmd = "fio"
            for key in self.opt:
                subcmd += f" --{key}"
                subcmd += f"={self.opt[key]}"

            for job in self.jobs:
                subcmd += job

            cmd.extend(["nohup", subcmd])
            lib.subproc.popen(cmd, True, False)
            return True
        except Exception as e:
            lib.printer.red(cmd)
            lib.printer.red(f"{__name__} [Error] {e}")
            return False
