#!/bin/bash

red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

# Update files
    # make

 echo ""
 echo "#####################################################"
 echo "###########   COMPILING SCALPA FILES    #############"
 echo "#####################################################"
 echo ""

 for file in `ls tests/auto`; do
  echo "----------  Compiling $file  ----------"
  ./scalpa tests/auto/$file
  echo ""

 done

 echo ""
 echo "#####################################################"
 echo "#############   RUN MIPS FILES    ###################"
 echo "#####################################################"
 echo ""

 # move mips files
 mkdir -p mips
 mv *.s mips/

 i=1
 total=15 # total number of tests

 for file in `ls mips`; do
   echo "----------  SPIM -f $file  ----------"
   spim -f mips/$file | tail -n +6 > tmp_res
   if cmp -s tmp_res tests/results/$file.res; then
        echo  "${green}>>>>>>>>>>> Test $i/$total mips/$file  PASSED ${reset} "
        ((i++))
   else
        echo "We are waiting results like :  "
        cat tests/results/$file.res
        echo "We got this  :  "
        cat tmp_res
        echo  "${red}>>>>>>>>>>> Test mips/$file FAILED ${reset}"
        exit 1 ;
   fi
   echo ""
 done

 rm tmp_res
