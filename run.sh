#!/bin/bash
 make
 for file in `ls tests`; do
  echo "Compiling File ----------> $file"
  ./scalpa tests/$file
  echo ""

 done

 # move mips files
 mkdir -p mips
 mv *.s mips/

 for file in `ls mips`; do
  echo "Run MIPS  -------------------> $file"
  spim -f mips/$file | tail -n +6
  echo ""
 done
