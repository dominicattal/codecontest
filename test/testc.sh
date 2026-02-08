#!/bin/bash
source test/config.sh
echo "Problem C tests"
echo "-----------------------------------------"
echo "Test: python accepted"
bin/client -i "$ip" -t $port -u "team1" -p "team1" -l "python3" -b "C" -f "test/problem3/test-py/accepted.py"
