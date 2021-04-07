import json

def get_response_code(result):
    data = json.loads(result)
    code = data['Response']['result']['status']['code']
    return code

def get_data_code(result):
    data = json.loads(result)
    code = data['Response']['result']['data']['returnCode']
    return code

def get_value_from_json(fileName, key):
    data = json.load(fileName)
    return data[key]

def get_list(src):
    data = json.loads(src)
    return data