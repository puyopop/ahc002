#!/bin/sh

make -p ./out

for file in `ls ./in/*`;
do
    ./main.out < $file > ./out/$(basename ${file})
done