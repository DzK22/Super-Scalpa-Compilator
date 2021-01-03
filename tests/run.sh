#!/bin/bash

red=`tput setaf 1`
green=`tput setaf 2`
yellow=`tput setaf 3`
reset=`tput sgr0`

mkdir -p tests/compil
rm tests/compil/* 2> /dev/null
rm tests/mips/* 2> /dev/null

# Update files
# make

echo ""
echo "#####################################################"
echo "###########   COMPILING SCALPA FILES    #############"
echo "#####################################################"
echo ""

for file in `ls tests/auto`; do
    echo "----------  Compiling $file  ----------"
    ./scalpa -o tests/mips/${file::-4} tests/auto/$file 2> tests/tmp_res

    if [ $? -eq "0" ]; then
        echo "" > tests/compil/$file.res
    else
        cat tests/tmp_res > tests/compil/$file.res
    fi

    echo ""

done

echo ""
echo "#####################################################"
echo "#############   RUN MIPS FILES    ###################"
echo "#####################################################"
echo ""

 # move mips files
 mkdir -p tests/mips

 i=1
 total=`ls tests/compil | wc -l` # total number of tests
 success=0

 for compfile in `ls tests/compil`; do
     file=${compfile::-8}.s

     if [ ! -e tests/compil/$compfile ] || [ $(cat tests/compil/$compfile | wc -c) -gt 1 ]; then
         cat tests/compil/$compfile
         echo  "${red}>>>>>>>>>>> Test $i/$total tests/compil/$compfile FAILED ${reset}"
     else
         echo "----------  SPIM -f $file  ----------"
         spim -f tests/mips/$file | grep "Loaded: /usr" --after-context=10000 | tail -n +2 > tests/tmp_res

         if cmp -s tests/tmp_res tests/results/$file.res; then
             echo  "${green}>>>>>>>>>>> Test $i/$total tests/mips/$file  PASSED ${reset} "
             ((success++))
         else
             echo "We are waiting results like :  "
             cat tests/results/$file.res
             echo -e "\nWe got this :  "
             cat tests/tmp_res
             echo  "${red}>>>>>>>>>>> Test $i/$total tests/mips/$file FAILED ${reset}"
         fi
     fi

     ((i++))
     echo ""
 done

 rm tests/tmp_res
 echo "${yellow}################################################"
 echo "#######  RESULTS: $(printf "%8s" "$success / $total") TESTS PASSED  #######"
 echo "################################################${reset}"
