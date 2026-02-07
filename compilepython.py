#!/bin/python3
import py_compile
import sys

try:
    # returns path to bytecode if successful
    res = py_compile.compile(sys.argv[1], cfile=sys.argv[2])
    if res == None:
        sys.exit(1)
except:
    sys.exit(1)
