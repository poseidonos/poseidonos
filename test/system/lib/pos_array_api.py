#!/usr/bin/env python3

import pos_array
import cli
import json

def get_all_arrays():
    arraylist = []
    out = cli.list_array()
    data = json.loads(out)
    for item in data['Response']['result']['data']['arrayList']:
        arr = pos_array.PosArray(item['name'], item['status'])
        arraylist.append(arr)
    return arraylist

def get_mounted_arrays():
    arraylist = []
    out = cli.list_array()
    data = json.loads(out)
    for item in data['Response']['result']['data']['arrayList']:
        if item['status'] == "Mounted":
            arr = pos_array.PosArray(item['name'], item['status'])
            arraylist.append(arr)
    return arraylist

def unmount_all_arrays():
    arraylist = get_mounted_arrays()
    for array in arraylist:
        cli.unmount_array(array)
    arraylist = get_mounted_arrays()
    if len(arraylist) == 0:
        return True
    return False