#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Invalid number of input"
  exit 1
fi

if [[ ! ($1 =~ ^[1-9][0-9]*$) ]] || [[ ! ($2 =~ ^[1-9][0-9]*$) ]]; then
  echo "Input must be positive integer"
  exit 1
fi

i=1

while [ $i -le $1 ]
do
  j=1

  while [ $j -le $2 ]
  do
    if [ $j -ne 1 ]; then
      printf "\t"
    fi

    printf "$i*$j=$((i*j))"
    j=$((j+1))
  done

  i=$((i+1))
  printf "\n"
done
