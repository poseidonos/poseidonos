#!/usr/bin/env python
#
# Copyright 2008 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Driver for starting up Google Mock class generator."""


import os, sys
import argparse

from cpp import gmock_class
from cpp import gmock_test

def parse_argument():
  parser = argparse.ArgumentParser()
  parser.add_argument("--root_directory", dest="root_directory", action="store", required=False, type=str, default="../../", help="poseidonos directory")
  parser.add_argument("file_or_directory", metavar="file_or_directory", type=str, nargs=1, help="file or directory to generate")
  args = parser.parse_args()
  return args


def main(args):
  dirname = os.path.abspath(args.root_directory + "/src")
  testdir = os.path.abspath(args.root_directory + "/test/unit-tests")
  desired = os.path.abspath(args.file_or_directory[0])
  isdir = False

  if os.path.isdir(desired):
    isdir = True
  elif os.path.isfile(desired):
    isdir = False
    sys.exit("Not support file update now.")
  else:
    sys.exit("Wrong file/directory name")
  
  # Create CMakeFile at testdir
  if not os.path.exists(testdir + '/CMakeLists.txt'):
    with open(testdir + '/CMakeLists.txt', "w") as f:
      f.write('POS_CMAKE_SUBDIR()')
  
  for dirpath, _, filenames in os.walk(dirname):
    if '/src/include' in dirpath:
      continue
    
    # scan all directories in src and create according directory to the unit-tests
    if not os.path.exists(testdir + dirpath.replace(dirname, "/")):
      os.makedirs(testdir + dirpath.replace(dirname, "/"))

    if (isdir) and (desired not in dirpath):
      continue

    # Remove the existing CMakeFiles 
    if os.path.exists(testdir + dirpath.replace(dirname, "") + '/CMakeLists.txt'):
      os.remove(testdir + dirpath.replace(dirname, "") + '/CMakeLists.txt')
    
    for filename in filenames:
      if ('.h' not in filename or '_mock.h' in filename or '.hpp' in filename):
        continue

      filepath = dirpath + '/' + filename
      try:
        # create mock file
        mocklines = gmock_class.run(filepath)
        if mocklines:
          # mock is okay to overwrite
          mockfile = testdir + dirpath.replace(dirname, "/") + '/' + filename.replace('.h', '_mock.h')
          with open(mockfile, "w") as f:
            f.write('\n'.join(mocklines))

        # create test file
        testlines = gmock_test.run(filepath)
        if testlines:
          # do not overwrite testfile
          testfile = testdir + dirpath.replace(dirname, "/") + '/' + filename.replace('.h', '_test.cpp')
          if not os.path.exists(testfile):
            with open(testfile, "w") as f:
              f.write('\n'.join(testlines))

        # create CMakeLists.txt file
        # ex) POS_ADD_TEST(journal_manager_ut journal_manager_test.cpp)
        if mocklines and testlines:
          cmakefile = testdir + dirpath.replace(dirname, "/") + '/' + 'CMakeLists.txt'
                
          with open(cmakefile, "a") as f:
            f.write('POS_ADD_UNIT_TEST(%s %s)\n' % (filename.replace('.h','_ut'),filename.replace('.h', '_test.cpp')))

      except:
        sys.stderr.write('Unable to create mock, test file of %s \n' % filepath)

if __name__ == '__main__':
  # Add the directory of this script to the path so we can import gmock_class.
  sys.path.append(os.path.dirname(__file__))
  args = parse_argument()
  main(args)


