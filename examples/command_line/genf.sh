#!/bin/bash

rm -rf tmp
mkdir tmp
cd tmp

let n=1
for ((i=1; i < 14; i++))
do
  let n=n*6
  ii=$(printf "%02d" $i)

  dd if=/dev/zero of=file_${ii}.img bs=1 count=0 seek=${n}
done

ls -Slh
