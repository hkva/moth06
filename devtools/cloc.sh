#!/bin/bash

find . -type f                      \
    \(                              \
        -name "*.cc"                \
        -or -name "*.hh"            \
        -or -name "*.py"            \
        -or -name "*.sh"            \
    \)                              \
    -and ! -path "*thirdparty*"     | xargs cloc
