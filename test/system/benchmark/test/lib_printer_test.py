import unittest

import lib
import os
import sys


class TestPrinter(unittest.TestCase):
    def test_red(self):
        filename = "TestPrinter.test_red.txt"
        file = open(filename, "w")
        sys.stdout = file
        lib.printer.red(f"test red color")
        sys.stdout = sys.__stdout__
        file.close()

        file = open(filename, "r")
        result = file.readline()
        file.close()
        os.system(f"rm {filename}")
        self.assertEqual(result, f"\033[1m\033[31mtest red color\033[0m\n")

    def test_green(self):
        filename = "TestPrinter.test_green.txt"
        file = open(filename, "w")
        sys.stdout = file
        lib.printer.green(f"test green color")
        sys.stdout = sys.__stdout__
        file.close()

        file = open(filename, "r")
        result = file.readline()
        file.close()
        os.system(f"rm {filename}")
        self.assertEqual(result, f"\033[1m\033[32mtest green color\033[0m\n")
