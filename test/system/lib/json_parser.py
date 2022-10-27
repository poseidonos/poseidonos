import json

def get_response_code(result):
    output = []
    code = 0
    output = result.splitlines()
    output_count = len(output)
    try:
        for i in range(0, output_count):
            data = json.loads(output[i])
            code |= data['Response']['result']['status']['code']
        return code
    except:
        return -1

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

def is_data_device(arrayinfo, devicename):
    data = json.loads(arrayinfo)
    for item in data['Response']['result']['data']['devicelist']:
        if item['name'] == devicename and item['type'] == "DATA":
            return True
    return False

def is_spare_device(arrayinfo, devicename):
    data = json.loads(arrayinfo)
    for item in data['Response']['result']['data']['devicelist']:
        if item['name'] == devicename and item['type'] == "SPARE":
            return True
    return False

def is_buffer_device(arrayinfo, devicename):
    data = json.loads(arrayinfo)
    for item in data['Response']['result']['data']['devicelist']:
        if item['name'] == devicename and item['type'] == "BUFFER":
            return True
    return False

def is_array_device(arrayinfo, devicename):
    data = json.loads(arrayinfo)
    for item in data['Response']['result']['data']['devicelist']:
        if item['name'] == devicename :
            return True
    return False

def is_system_device(listdev, devicename):
    data = json.loads(listdev)
    for item in data['Response']['result']['data']['devicelist']:
        if item['name'] == devicename and item['class'] == "SYSTEM":
            return True
    return False

def is_device_exists(listdev, devicename):
    data = json.loads(listdev)
    for item in data['Response']['result']['data']['devicelist']:
        if item['name'] == devicename:
            return True
    return False

def is_ssd(listdev, devicename):
    data = json.loads(listdev)
    for item in data['Response']['result']['data']['devicelist']:
        if item['name'] == devicename and item['type'] == "SSD":
            return True
    return False


def make_result_code(result):
    return json.dumps({"Response":{"result":{"status":{"code":result}}}})
