#!/bin/bash
name="result"

for i in script1_test*.txt; 
do
    ./testProc ${i} > out
    #cat out
    #cat ${i:0:8}${name}${i:12}
    if ! cmp -s out ${i:0:8}${name}${i:12}; then
        echo ${i} : BAD
    fi
done

