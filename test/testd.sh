#!/bin/bash
source test/config.sh
echo "Problem D tests"
echo "-----------------------------------------"
echo "Test: python accepted"
bin/client -i "$ip" -t $port -u "team1" -p "team1" -l "python3" -b "D" -f "test/problem4/test-py/accepted1.py"
echo "-----------------------------------------"
echo "Test: python accepted binary search"
bin/client -i "$ip" -t $port -u "team1" -p "team1" -l "python3" -b "D" -f "test/problem4/test-py/accepted2.py"
echo "-----------------------------------------"
