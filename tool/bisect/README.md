## bisect_test.py - Bug commit finder
Find first bad commit easily via bisect feature!

## Usage
bisect_test.py [-h] [-b BAD] [-g GOOD] -p PATH

## Optional Arguments
    -h, --help            show this help message and exit
    -b BAD, --bad BAD     set first bad commit or tag, default: HEAD
    -g GOOD, --good GOOD  set last good commit or tag, default: pos-0.5.2
    -p PATH, --path PATH  set test script path. Test should return 0 or 1 at the
                          case of fail or success, respectively.

## Example
1) When you already found that test_example.py is success at pos-0.5.1 and failed at 708a598a613 then,
```bash
./bisect_test.py -g pos-0.5.1 -b 708a598a613 -p ./test_example.py
```
