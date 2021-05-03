#!/bin/sh

count=0;
total=0; 

for file in $(ls ./in/*);
do
    total=$(echo $(./main.out < ${file} 2>/dev/null | wc -c)+${total} | bc)
    ((count++))
done
echo "$total / $count" | bc
