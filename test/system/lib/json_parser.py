import json

def get_response_code(result):
    data = json.loads(result)
    code = data['Response']['result']['status']['code']
    return code

def get_data_code(result):
    data = json.loads(result)
    code = data['Response']['result']['data']['returnCode']
    return code

def get_data(result):
    data = json.loads(result)
    code = data['Response']['result']['data']
    return code

def get_value_from_json(fileName, key):
    data = json.load(fileName)
    return data[key]

def get_list(src):
    data = json.loads(src)
    return data

def get_state(src):
    data = json.loads(src)
    state = data['Response']['info']['state']
    return state

def get_capacity(src):
    data = json.loads(src)
    capa = data['Response']['info']['capacity']
    return capa;

def get_used(src):
    data = json.loads(src)
    used = data['Response']['info']['used']
    return used;

def is_online(src):
    data = json.loads(src)
    state = data['Response']['info']['state']
    if state == "NORMAL" or state == "DEGRADED" or state == "REBUILD":
        return True
    else:
        return False