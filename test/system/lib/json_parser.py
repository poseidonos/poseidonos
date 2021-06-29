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

def get_state(arrayinfo):
    data = json.loads(arrayinfo)
    state = data['Response']['result']['data']['state']
    return state

def get_situation(arrayinfo):
    data = json.loads(arrayinfo)
    situ = data['Response']['result']['data']['situation']
    return situ

def get_capacity(arrayinfo):
    data = json.loads(arrayinfo)
    capa = data['Response']['result']['data']['capacity']
    return capa

def get_used(arrayinfo):
    data = json.loads(arrayinfo)
    used = data['Response']['result']['data']['used']
    return used

def is_online(arrayinfo):
    state = get_state(arrayinfo)
    if state == "NORMAL" or state == "BUSY":
        return True
    else:
        return False