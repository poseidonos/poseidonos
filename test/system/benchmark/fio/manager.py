import asyncio
import copy
import lib


class Fio:
    def __init__(self, id, pw, nic_ssh):
        self.id = id
        self.pw = pw
        self.nic_ssh = nic_ssh
        self.opt = {}
        self.jobs = []
        self.cmd = ""

    def Prepare(self):
        try:
            self.cmd = f"sshpass -p {self.pw} ssh {self.id}@{self.nic_ssh} nohup 'fio"
            for key in self.opt:
                self.cmd += f" --{key}={self.opt[key]}"
            for job in self.jobs:
                self.cmd += job
            self.cmd += f" > {self.opt['output']}.eta"
            self.cmd += "'"
            return True
        except Exception as e:
            lib.printer.red(f"{__name__} [Error] {e}")
            return False


def parallel_run(cmd_list):
    asyncio.set_event_loop(asyncio.new_event_loop())
    loop = asyncio.get_event_loop()
    tasks = asyncio.gather(*[
        lib.subproc.async_run(cmd, True) for cmd in cmd_list
    ])
    loop.run_until_complete(tasks)
    loop.close()
