#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Invalid number of input"
  exit 1
fi

if [[ ! ($1 =~ ^[1-9][0-9]*$) ]] || [[ ! ($2 =~ ^[1-9][0-9]*$) ]]; then
  echo "Input must be positive integer"
  exit 1
fi
