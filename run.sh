#!/bin/bash
 for file in `ls tests`; do
  echo "Compiling File ----------> $file"
  ./cs.out tests/$file
  echo ""

 done

 for file in `ls *.s`; do
  echo "Run MIPS  -------------------> $file"
  spim -f $file | tail -n +6
  echo ""
 done
