import os

def expect_true(code):
    if code == 0:
        return "pass"
    return "fail"

def expect_false(code):
    if code != 0:
        return "pass"
    return "fail"

def clear_result(file):
    resultFile = file + ".result"
    if os.path.exists(resultFile):
        os.remove(resultFile)