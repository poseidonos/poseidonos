def expect_true(code):
    if code == 0:
        return "pass"
    return "fail"

def expect_false(code):
    if code != 0:
        return "pass"
    return "fail"