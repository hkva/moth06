#!/bin/bash

find . -type f                      \
    \(                              \
        -name "*.cc"                \
        -or -name "*.hh"            \
        -or -name "*.py"            \
        -or -name "*.sh"            \
        -or -name "CMakeLists.txt"  \
    \)                              \
    -and ! -path "*thirdparty*"     | xargs cloc
