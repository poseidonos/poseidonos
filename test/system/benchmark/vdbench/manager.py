import lib
import csv
import json
import asyncio


class Vdbench:
    def __init__(self, init_name, id, pw, nic_ssh, vdbench_dir, output_dir, jvm=32):
        self.init_name = init_name
        self.id = id
        self.pw = pw
        self.nic_ssh = nic_ssh
        self.opt = {}  # size, openflags
        self.jobs = []
        self.cmd = ""
        self.vdfile = "test.vd"
        self.vdbench_dir = vdbench_dir
        self.jvm = jvm
        self.column_list = ["timestamp", "rate", "MB/sec", "resp", "read_resp", "write_resp"]
        self.output_dir = output_dir

    def CreateVdFile(self, initiators, rd_list, rd_idx=0, mulit_host=False):
        create_cmd = ["sshpass", "-p", self.pw, "ssh", f"{self.id}@{self.nic_ssh}"]
        create_cmd.extend(["echo", f"hd=default,jvms={self.jvm}", ">", f"{self.vdbench_dir}/{self.vdfile};"])
        remote_cnt = 0
        if len(initiators) > 1:
            for key in initiators:
                init = initiators[key]
                if remote_cnt == 0:
                    create_cmd.extend(["echo", "hd=localhost", ">>", f"{self.vdbench_dir}/{self.vdfile};"])
                    remote_cnt += 1
                    continue
                create_cmd.extend(["echo", f"hd={init.name},shell=ssh,system={init.nic_ssh},vdbench={init.vdbench_dir},user={init.id}", ">>", f"{self.vdbench_dir}/{self.vdfile};"])
        vd_disk_names = {}
        remote_cnt = 0
        for key in initiators:
            init = initiators[key]
            vd_disk_name = {}
            if remote_cnt == 0:
                hostname = "localhost"
                remote_cnt += 1
            else:
                hostname = f"{init.name}"
            for diskname in init.device_list:
                sd_device_name = diskname.split('/')
                vd_disk_name[diskname] = str(sd_device_name[2])
                create_cmd.extend(["echo", f"sd={vd_disk_name[diskname]}_{init.name},host={hostname},lun={diskname},openflags=o_direct,size={self.opt['size']}"])
                create_cmd.extend([">>", f"{self.vdbench_dir}/{self.vdfile};"])

            vd_disk_names[key] = vd_disk_name

        create_cmd.extend(["echo", rf"wd=seq,sd=nvme\*,xfersize=4k,rdpct=0,seekpct=0", ">>", f"{self.vdbench_dir}/{self.vdfile};"])
        create_cmd.extend(["echo", rf"wd=rand,sd=nvme\*,xfersize=4k,rdpct=0,seekpct=100", ">>", f"{self.vdbench_dir}/{self.vdfile};"])

        if rd_idx == -1:
            for rd in rd_list:
                create_cmd.extend(["echo", f"rd={rd}", ">>", f"{self.vdbench_dir}/{self.vdfile};"])
        else:
            create_cmd.extend(["echo", f"rd={rd_list[rd_idx]}", ">>", f"{self.vdbench_dir}/{self.vdfile};"])

        lib.subproc.sync_run(create_cmd, False, False)
        return vd_disk_names

    def run(self, sync=False):
        run_cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} "
        run_cmd += f"sudo nohup {self.vdbench_dir}/vdbench -f {self.vdbench_dir}/{self.vdfile} -o {self.vdbench_dir}/result_qos"
        if sync is True:
            lib.subproc.sync_run(run_cmd)
        else:
            run_cmd += "&"
            lib.subproc.sync_run(run_cmd)

    def CopyVdbenchTotalResult(self, print_average, workload):
        for wl in workload:
            # Get total result to csv file
            result_cmd = f"sshpass -p {self.pw} ssh -o StrictHostKeyChecking=no {self.id}@{self.nic_ssh} "
            result_cmd += f"sudo nohup {self.vdbench_dir}/vdbench parseflat -i {self.vdbench_dir}/result_qos/flatfile.html -c"
            for cl in self.column_list:
                result_cmd += f" {cl}"
            result_cmd += f" -f Run {wl} -o {self.vdbench_dir}/{wl}.csv"
            if print_average is True:
                result_cmd += " -a"
            lib.subproc.sync_run(result_cmd, True)

            copy_cmd = f"sshpass -p {self.pw} scp {self.id}@{self.nic_ssh}:{self.vdbench_dir}/{wl}.csv ./"
            lib.subproc.sync_run(copy_cmd)

            # Convert csv file to json
            csv_file_name = f"./{wl}.csv"
            json_file_name = f"./{self.output_dir}/{wl}.json"
            jsonArray = []
            with open(csv_file_name, encoding='utf-8') as csvf:
                csvReader = csv.DictReader(csvf)
                for row in csvReader:
                    jsonArray.append(row)
            with open(json_file_name, 'w', encoding='utf-8') as jsonf:
                jsonString = json.dumps(jsonArray, indent=4)
                jsonf.write(jsonString)
            delete_cmd = f"rm -rf ./{wl}.csv"
            lib.subproc.sync_run(delete_cmd)

    def GetBasePerformance(self, workload):
        # total_result: {"seq_r": {"bw": 10, "iops": 20}, "rand_w": {"bw": 20, "iops": 30}}
        total_result = {}
        for wl in workload:
            result = {}
            json_config_file = f"./{self.output_dir}/{wl}.json"
            with open(json_config_file, "r") as f:
                json_array = json.load(f)
            result["iops"] = float(json_array[0]["rate"])
            result["bw"] = float(json_array[0]["MB/sec"])
            total_result[wl] = result
        return total_result

    def ConvertCsvToJson(self, file_name, wl):
        csv_file_name = f"./{file_name}.csv"
        json_file_name = f"./{self.output_dir}/{file_name}_{wl}.json"
        jsonArray = []
        with open(csv_file_name, encoding='utf-8') as csvf:
            csvReader = csv.DictReader(csvf)
            for row in csvReader:
                jsonArray.append(row)
            del jsonArray[-1]
        with open(json_file_name, 'w', encoding='utf-8') as jsonf:
            jsonString = json.dumps(jsonArray, indent=4)
            jsonf.write(jsonString)

        delete_cmd = f"rm -rf ./{file_name}.csv"
        lib.subproc.sync_run(delete_cmd)
        return json_file_name

    def CopyHtmlResult(self, file_name, init_name):
        file_name = file_name + "_" + init_name
        html_file = file_name + ".html"
        copy_cmd = f"sshpass -p {self.pw} scp {self.id}@{self.nic_ssh}:{self.vdbench_dir}/result_qos/{html_file} ./"
        lib.subproc.sync_run(copy_cmd)
        return file_name

    def ParseHtmlResult(self, file_name, workload_name):
        text = []
        f = open(f"./{file_name}.html", 'r')

        line = f.readline()
        while "interval" not in line:
            if not line:
                break
            line = f.readline()

        text.append(line.split())
        line = f.readline()

        while True:
            line = f.readline()
            if not line:
                break
            text.append(line.split())

        del_list = [0, 2, 3, 6, 7, 8, 9, 10]
        result = []
        for j in text[1:]:
            if not j:
                continue
            row_result = []
            for i in del_list:
                row_result.append(j[i])
            result.append(row_result)

        with open(f"{file_name}.csv", 'w', newline='') as csvfile:
            wr = csv.writer(csvfile)
            wr.writerow(self.column_list)
            for row in result:
                wr.writerow(row)

        delete_cmd = f"rm -rf ./{file_name}.html"
        lib.subproc.sync_run(delete_cmd)
        return self.ConvertCsvToJson(file_name, workload_name)
