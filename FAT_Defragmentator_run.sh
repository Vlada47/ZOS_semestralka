#!/bin/bash

program="./"$1
input_file=$2
output_file=$3
max_thread_cnt=$4

if [ $# -eq 4 ] ; then

	if [ $4 -ge 1 ] ; then
		
		echo "" > FAT_Defragmentator_log.txt

		for (( i=1; i<$max_thread_cnt+1; i++ ))
		do
			 echo "Beh s $i vlakny..." >> FAT_Defragmentator_log.txt
			 { time $program $input_file $output_file $i 2>&1 > /dev/null ; } >> FAT_Defragmentator_log.txt 2>&1
			 echo "" >> FAT_Defragmentator_log.txt
		done
		
	else
		echo "Pocet vlaken musi byt cele kladne cislo! Vas vstup: $4"
	fi
else
	echo "Musite zadat vsechny parametry:"
	echo "nazev programu ke spusteni"
	echo "cestu ke vstupni FAT"
	echo "cestu k vystupni FAT"
	echo "maximalni pocet vlaken"
fi