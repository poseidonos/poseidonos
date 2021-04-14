#!/bin/bash

mkdir -p ../build/

echo "Generating test skeletons and mocks..."
cd generator/
./generator.py --root_directory ../../ ../../src/ &> ../build/generator.log

#echo "Applying cpp_formatter.py..."
#../../script/cpp_formatter.py -d ../unit-tests &> ../build/cpp_formatter.log
#commented out since it takes 10+ seconds to run
#alternative: use git-clang-format. But, this applies to staged/commited files only. For now, I will keep "gen_ut" that includes cpp_formatter.py script.
