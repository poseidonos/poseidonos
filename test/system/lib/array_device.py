class ArrayDevice(object):
    def __init__(self, name, type, *args, **kwargs):
        self.name = name
        self.type = type

class ArrayDevice():
    def __init__(self, name, type):
        self.name = name
        self.type = type