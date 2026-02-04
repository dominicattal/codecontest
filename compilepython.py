import py_compile
import sys

try:
    # returns path to bytecode if successful
    print("??")
    res = py_compile.compile(sys.argv[1], cfile=sys.argv[2])
    print("??")
    if res == None:
        sys.exit(1)
except:
    sys.exit(1)
