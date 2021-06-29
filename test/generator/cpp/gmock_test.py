#!/usr/bin/env python
#
# Copyright 2008 Google Inc.  All Rights Reserved.
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

"""Generate Google Mock classes from base classes.

This program will read in a C++ source file and output the Google Mock
classes for the specified classes.  If no class is specified, all
classes in the source file are emitted.

Usage:
  gmock_class.py header-file.h [ClassName]...

Output is sent to stdout.
"""

import os
import re
import sys

from cpp import ast
from cpp import utils

# Preserve compatibility with Python 2.3.
try:
  _dummy = set
except NameError:
  import sets

  set = sets.Set

_VERSION = (1, 0, 1)  # The version of this script.
# How many spaces to indent.  Can set me with the INDENT environment variable.
_INDENT = 4

def _GenerateMethodTests(output_lines, source, class_node):
  '''
  # These are not needed now, but can be used in future
  function_type = (
      ast.FUNCTION_VIRTUAL | ast.FUNCTION_PURE_VIRTUAL | ast.FUNCTION_OVERRIDE)
  ctor_or_dtor = ast.FUNCTION_CTOR | ast.FUNCTION_DTOR
  indent = ' ' * _INDENT
  '''
  method_list = []

  # in the case of operator overloading, we'd like to ignore.
  exclude_nodes = set(["==", "<", ">"])
  for node in class_node.body:
    # TODO(kyuho): check about virtual, ctor, ...
    #if (isinstance(node, ast.Function) and node.modifiers & function_type and
    #    not node.modifiers & ctor_or_dtor):
    if (isinstance(node, ast.Function)):
      if (node.name in method_list):
        continue
      if (node.name.strip() in exclude_nodes):
        continue
      method_list.append(node.name)
      
      output_lines.extend([
        'TEST(%s, %s_) {' % (class_node.name, node.name), '', '}', ''
      ])      

def _GenerateTests(filename, source, ast_list):
  processed_class_names = set()
  lines = []

  # Add header file before the namespace
  try:
    lines.extend(['#include <gtest/gtest.h>'])
    src_index = filename.split('/').index('src')
    lines.extend(['#include "%s"' % '/'.join(filename.split('/')[src_index:])])
    lines.extend(' ')
  except:
    print "Error occured at include header file"
    return lines


  for node in ast_list:
    if (isinstance(node, ast.Class) and node.body):
      class_name = node.name
      parent_name = class_name
      processed_class_names.add(class_name)
      class_node = node
      # Add namespace before the class.
      if class_node.namespace:
        lines.extend(['namespace %s {' % n for n in class_node.namespace])  # }
        lines.append('')

      # Add all the methods
      _GenerateMethodTests(lines, source, class_node)

      # Close the namespace.
      if class_node.namespace:
        for i in range(len(class_node.namespace) - 1, -1, -1):
          lines.append('}  // namespace %s' % class_node.namespace[i])
        lines.append('')  # Add an extra newline.
        
  return lines


def run(filepath):
  global _INDENT
  try:
    _INDENT = int(os.environ['INDENT'])
  except KeyError:
    pass
  except:
    sys.stderr.write('Unable to use indent of %s\n' % os.environ.get('INDENT'))
  desired_class_names = None  # None means all classes in the source file.
  source = utils.ReadFile(filepath)
  if source is None:
    return 1
  
  builder = ast.BuilderFromSource(source, filepath)
  try:
    entire_ast = filter(None, builder.Generate())
  except KeyboardInterrupt:
    return
  except:
    # An error message was already printed since we couldn't parse.
    sys.exit(1)
  else:
    lines = _GenerateTests(filepath, source, entire_ast)
    return lines



